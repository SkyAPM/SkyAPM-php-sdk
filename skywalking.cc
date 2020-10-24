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

#include <src/sky_shm.h>
#include "php_skywalking.h"

#include "src/sky_module.h"
#include "src/segment.h"
#include "sys/mman.h"

#ifdef MYSQLI_USE_MYSQLND
#include "ext/mysqli/php_mysqli_structs.h"
#endif

ZEND_DECLARE_MODULE_GLOBALS(skywalking)

struct service_info *s_info = nullptr;
struct sky_shm_obj *sky_shm = nullptr;

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
	STD_PHP_INI_ENTRY("skywalking.grpc_tls_enable", "0", PHP_INI_ALL, OnUpdateBool, grpc_tls_enable, zend_skywalking_globals, skywalking_globals)
	STD_PHP_INI_ENTRY("skywalking.grpc_tls_pem_root_certs", "", PHP_INI_ALL, OnUpdateString, grpc_tls_pem_root_certs, zend_skywalking_globals, skywalking_globals)
	STD_PHP_INI_ENTRY("skywalking.grpc_tls_pem_private_key", "", PHP_INI_ALL, OnUpdateString, grpc_tls_pem_private_key, zend_skywalking_globals, skywalking_globals)
	STD_PHP_INI_ENTRY("skywalking.grpc_tls_pem_cert_chain", "", PHP_INI_ALL, OnUpdateString, grpc_tls_pem_cert_chain, zend_skywalking_globals, skywalking_globals)
PHP_INI_END()

static void php_skywalking_init_globals(zend_skywalking_globals *skywalking_globals) {
    skywalking_globals->app_code = nullptr;
    skywalking_globals->enable = 0;
    skywalking_globals->version = 0;
    skywalking_globals->grpc = nullptr;
    skywalking_globals->authentication = nullptr;

    // tls
    skywalking_globals->grpc_tls_enable = 0;
    skywalking_globals->grpc_tls_pem_root_certs = nullptr;
    skywalking_globals->grpc_tls_pem_private_key = nullptr;
    skywalking_globals->grpc_tls_pem_cert_chain = nullptr;
}

PHP_FUNCTION (skywalking_trace_id) {
    if (SKYWALKING_G(enable) && SKYWALKING_G(segment) != nullptr) {
        auto *segment = static_cast<Segment *>(SKYWALKING_G(segment));
        std::string trace_id = segment->getTraceId();
        RETURN_STRING(trace_id.c_str());
    } else {
        RETURN_STRING("");
    }
}

PHP_MINIT_FUNCTION (skywalking) {
	ZEND_INIT_MODULE_GLOBALS(skywalking, php_skywalking_init_globals, NULL);
	REGISTER_INI_ENTRIES();

	if (SKYWALKING_G(enable)) {

        int protection = PROT_READ | PROT_WRITE;
        int visibility = MAP_SHARED | MAP_ANONYMOUS;

        s_info = (struct service_info *) mmap(nullptr, sizeof(struct service_info), protection, visibility, -1, 0);

	    sky_shm = (struct sky_shm_obj *) malloc(sizeof(struct sky_shm_obj));
        sky_module_init();
	}

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION (skywalking) {
    UNREGISTER_INI_ENTRIES();
    if (SKYWALKING_G(enable)) {

        if (sky_shm->shm_id > 0) {
            std::string msg("terminate");
            char *buf = new char[msg.length() + 1];
            strcpy(buf, msg.c_str());

            sky_sem_p(sky_shm->sem_id);
            sky_shm_write(sky_shm->shm_addr, buf);
            sky_sem_v(sky_shm->sem_id);

            sky_shm_del(sky_shm->shm_id);

            delete [] buf;
        }

        if (sky_shm->shm_id > 0) {
            sky_sem_del(sky_shm->sem_id);
        }

        free(sky_shm);
        sky_shm = nullptr;
    }
    return SUCCESS;
}

PHP_RINIT_FUNCTION(skywalking)
{

#if defined(COMPILE_DL_SKYWALKING) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
    if (SKYWALKING_G(enable)) {
        if (cli_debug == 1) {
            strcpy(s_info->service, "service");
            strcpy(s_info->service_instance, "service_instance");
        }

        if (strcasecmp("fpm-fcgi", sapi_module.name) == 0 || (strcasecmp("cli", sapi_module.name) == 0 && cli_debug == 1)) {
            if (strlen(s_info->service_instance) == 0) {
                return SUCCESS;
            }

            sky_request_init(nullptr);
        }
    }
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(skywalking)
{
	if (SKYWALKING_G(enable)) {
        if (strcasecmp("fpm-fcgi", sapi_module.name) == 0 || (strcasecmp("cli", sapi_module.name) == 0 && cli_debug == 1)) {
            if (SKYWALKING_G(segment) == nullptr) {
                return SUCCESS;
            }

            sky_request_flush(nullptr);
            zval_dtor(&SKYWALKING_G(curl_header));
        }
	}
	return SUCCESS;
}

PHP_MINFO_FUNCTION(skywalking)
{
	DISPLAY_INI_ENTRIES();
}

//PHP_GINIT_FUNCTION(skywalking)
//{
//    memset(skywalking_globals, 0, sizeof(*skywalking_globals));
//}

zend_module_dep skywalking_deps[] = {
        ZEND_MOD_REQUIRED("json")
        ZEND_MOD_REQUIRED("pcre")
        ZEND_MOD_REQUIRED("standard")
        ZEND_MOD_REQUIRED("curl")
        ZEND_MOD_END
};

const zend_function_entry skywalking_functions[] = {
        PHP_FE (skywalking_trace_id, NULL)
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