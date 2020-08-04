/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2017 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "php_skywalking.h"
#include <string.h>
#include <unistd.h>
#include "main/SAPI.h" /* for sapi_module */
#include <zend_interfaces.h>

#include "sys/mman.h"
#include "src/manager_wrapper.h"
#include "src/segment_wrapper.h"
#include "src/span_wrapper.h"
#include "src/tag_wrapper.h"

#include "src/sky_utils.h"
#include "src/sky_curl.h"
#include "src/sky_execute.h"

#ifdef MYSQLI_USE_MYSQLND
#include "ext/mysqli/php_mysqli_structs.h"
#endif

extern void (*ori_execute_ex)(zend_execute_data *execute_data);

extern void (*ori_execute_internal)(zend_execute_data *execute_data, zval *return_value);

extern void (*orig_curl_exec)(INTERNAL_FUNCTION_PARAMETERS);

extern void (*orig_curl_setopt)(INTERNAL_FUNCTION_PARAMETERS);

extern void (*orig_curl_setopt_array)(INTERNAL_FUNCTION_PARAMETERS);

extern void (*orig_curl_close)(INTERNAL_FUNCTION_PARAMETERS);

ZEND_DECLARE_MODULE_GLOBALS(skywalking)

struct service_info *s_info;
static int fd[2];

#if SKY_DEBUG
static int cli_debug = 1;
#else
static int cli_debug = 0;
#endif


PHP_INI_BEGIN()
#if SKY_DEBUG
	STD_PHP_INI_BOOLEAN("skywalking.enable", "1", PHP_INI_ALL, OnUpdateBool, enable, zend_skywalking_globals, skywalking_globals)
#else
	STD_PHP_INI_BOOLEAN("skywalking.enable", "0", PHP_INI_ALL, OnUpdateBool, enable, zend_skywalking_globals, skywalking_globals)
#endif
	STD_PHP_INI_ENTRY("skywalking.version", "8", PHP_INI_ALL, OnUpdateLong, version, zend_skywalking_globals, skywalking_globals)
	STD_PHP_INI_ENTRY("skywalking.app_code", "hello_skywalking", PHP_INI_ALL, OnUpdateString, app_code, zend_skywalking_globals, skywalking_globals)
	STD_PHP_INI_ENTRY("skywalking.authentication", "", PHP_INI_ALL, OnUpdateString, authentication, zend_skywalking_globals, skywalking_globals)
	STD_PHP_INI_ENTRY("skywalking.grpc", "127.0.0.1:11800", PHP_INI_ALL, OnUpdateString, grpc, zend_skywalking_globals, skywalking_globals)
PHP_INI_END()

static void php_skywalking_init_globals(zend_skywalking_globals *skywalking_globals) {
    skywalking_globals->app_code = NULL;
    skywalking_globals->enable = 0;
    skywalking_globals->version = 0;
    skywalking_globals->grpc = NULL;
    skywalking_globals->authentication = NULL;
}

PHP_MINIT_FUNCTION (skywalking) {
	ZEND_INIT_MODULE_GLOBALS(skywalking, php_skywalking_init_globals, NULL);
	REGISTER_INI_ENTRIES();

	if (SKYWALKING_G(enable)) {
        if (strcasecmp("cli", sapi_module.name) == 0 && cli_debug == 0) {
            return SUCCESS;
        }

        ori_execute_ex = zend_execute_ex;
        zend_execute_ex = sky_execute_ex;

        ori_execute_internal = zend_execute_internal;
        zend_execute_internal = sky_execute_internal;

		// bind curl
		zend_function *old_function;
		if ((old_function = zend_hash_str_find_ptr(CG(function_table), "curl_exec", sizeof("curl_exec") - 1)) != NULL) {
			orig_curl_exec = old_function->internal_function.handler;
			old_function->internal_function.handler = sky_curl_exec_handler;
		}
        if ((old_function = zend_hash_str_find_ptr(CG(function_table), "curl_setopt", sizeof("curl_setopt")-1)) != NULL) {
            orig_curl_setopt = old_function->internal_function.handler;
            old_function->internal_function.handler = sky_curl_setopt_handler;
        }
        if ((old_function = zend_hash_str_find_ptr(CG(function_table), "curl_setopt_array", sizeof("curl_setopt_array")-1)) != NULL) {
            orig_curl_setopt_array = old_function->internal_function.handler;
            old_function->internal_function.handler = sky_curl_setopt_array_handler;
        }
        if ((old_function = zend_hash_str_find_ptr(CG(function_table), "curl_close", sizeof("curl_close")-1)) != NULL) {
            orig_curl_close = old_function->internal_function.handler;
            old_function->internal_function.handler = sky_curl_close_handler;
        }

        if (pipe(fd) == 0) {
            int protection = PROT_READ | PROT_WRITE;
            int visibility = MAP_SHARED | MAP_ANONYMOUS;

            s_info = (struct service_info *) mmap(NULL, sizeof(struct service_info), protection, visibility, -1, 0);

            manager_init(SKYWALKING_G(version), SKYWALKING_G(app_code), SKYWALKING_G(grpc), s_info, fd);
        }
	}

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(skywalking)
{
	UNREGISTER_INI_ENTRIES();
    return SUCCESS;
}

PHP_RINIT_FUNCTION(skywalking)
{

#if defined(COMPILE_DL_SKYWALKING) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
    if (SKYWALKING_G(enable)) {
        if (strcasecmp("cli", sapi_module.name) == 0 && cli_debug == 0) {
            return SUCCESS;
        }

        if (cli_debug == 1) {
            strcpy(s_info->service, "service");
            strcpy(s_info->service_instance, "service_instance");
        }

        if (strlen(s_info->service_instance) == 0) {
            return SUCCESS;
        }

        array_init(&SKYWALKING_G(curl_header));

        zval *carrier = NULL;
        zval *sw;

        zend_bool jit_initialization = PG(auto_globals_jit);

        if (jit_initialization) {
            zend_string *server_str = zend_string_init("_SERVER", sizeof("_SERVER") - 1, 0);
            zend_is_auto_global(server_str);
            zend_string_release(server_str);
        }
        carrier = zend_hash_str_find(&EG(symbol_table), ZEND_STRL("_SERVER"));

        if (SKYWALKING_G(version) == 5) {
            sw = zend_hash_str_find(Z_ARRVAL_P(carrier), "HTTP_SW3", sizeof("HTTP_SW3") - 1);
        } else if (SKYWALKING_G(version) == 6 || SKYWALKING_G(version) == 7) {
            sw = zend_hash_str_find(Z_ARRVAL_P(carrier), "HTTP_SW6", sizeof("HTTP_SW6") - 1);
        } else if (SKYWALKING_G(version) == 8) {
            sw = zend_hash_str_find(Z_ARRVAL_P(carrier), "HTTP_SW8", sizeof("HTTP_SW8") - 1);
        } else {
            sw = NULL;
        }

        SKYWALKING_G(segment) = segment_init(s_info->service, s_info->service_instance, SKYWALKING_G(version), sw != NULL ? Z_STRVAL_P(sw) : "");

        // init entry span
        void *span = segment_create_span(SKYWALKING_G(segment), SPAN_TYPE_ENTRY);
        char *uri = get_page_request_uri();
        char *peer = get_page_request_peer();

        span_set_operation_name(span, (uri == NULL) ? "" : uri);
        span_set_peer(span, (peer == NULL) ? "" : peer);
        span_set_span_layer(span, SPAN_LAYER_HTTP);

        void *tag = tag_init("url", (uri == NULL) ? "" : uri);
        span_put_tag(span, tag);

        if (SKYWALKING_G(version) == 8) {
            span_set_component_id(span, 8001);
        } else {
            span_set_component_id(span, 49);
        }

        segment_create_refs(SKYWALKING_G(segment));

        if (peer != NULL) {
            efree(peer);
        }
        if (uri != NULL) {
            efree(uri);
        }
    }
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(skywalking)
{
	if(SKYWALKING_G(enable)){
        if (strcasecmp("cli", sapi_module.name) == 0 && cli_debug == 0) {
            return SUCCESS;
        }
        if (SKYWALKING_G(segment) == NULL) {
            return SUCCESS;
        }

        char *msg = segment_marshal(SKYWALKING_G(segment), SG(sapi_headers).http_response_code);
        segment_free(SKYWALKING_G(segment));
        write(fd[1], msg, strlen(msg));
        zval_dtor(&SKYWALKING_G(curl_header));
	}
	return SUCCESS;
}

PHP_MINFO_FUNCTION(skywalking)
{
	DISPLAY_INI_ENTRIES();
}

PHP_GINIT_FUNCTION(skywalking)
{
    memset(skywalking_globals, 0, sizeof(*skywalking_globals));
}

zend_module_dep skywalking_deps[] = {
        ZEND_MOD_REQUIRED("json")
        ZEND_MOD_REQUIRED("pcre")
        ZEND_MOD_REQUIRED("standard")
        ZEND_MOD_REQUIRED("curl")
        ZEND_MOD_END
};

const zend_function_entry skywalking_functions[] = {
        PHP_FE_END
};

zend_module_entry skywalking_module_entry = {
        STANDARD_MODULE_HEADER_EX,
        NULL,
        skywalking_deps,
        "skywalking",
        skywalking_functions,
        PHP_MINIT(skywalking),
        PHP_MSHUTDOWN(skywalking),
        PHP_RINIT(skywalking),
        PHP_RSHUTDOWN(skywalking),
        PHP_MINFO(skywalking),
        PHP_SKYWALKING_VERSION,
        STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_SKYWALKING
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(skywalking)
#endif