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
#include <fstream>
#include "php_skywalking.h"

#include "src/sky_utils.h"
#include "src/sky_module.h"
#include "src/segment.h"
#include "sys/mman.h"

#ifdef MYSQLI_USE_MYSQLND
#include "ext/mysqli/php_mysqli_structs.h"
#endif

ZEND_DECLARE_MODULE_GLOBALS(skywalking)

struct service_info *s_info = nullptr;

PHP_INI_BEGIN()
    STD_PHP_INI_BOOLEAN("skywalking.enable", "0", PHP_INI_ALL, OnUpdateBool, enable, zend_skywalking_globals, skywalking_globals)
	STD_PHP_INI_ENTRY("skywalking.version", "8", PHP_INI_ALL, OnUpdateLong, version, zend_skywalking_globals, skywalking_globals)
	STD_PHP_INI_ENTRY("skywalking.app_code", "hello_skywalking", PHP_INI_ALL, OnUpdateString, app_code, zend_skywalking_globals, skywalking_globals)
	STD_PHP_INI_ENTRY("skywalking.authentication", "", PHP_INI_ALL, OnUpdateString, authentication, zend_skywalking_globals, skywalking_globals)
	STD_PHP_INI_ENTRY("skywalking.grpc", "127.0.0.1:11800", PHP_INI_ALL, OnUpdateString, grpc, zend_skywalking_globals, skywalking_globals)
	STD_PHP_INI_BOOLEAN("skywalking.grpc_tls_enable", "0", PHP_INI_ALL, OnUpdateBool, grpc_tls_enable, zend_skywalking_globals, skywalking_globals)
	STD_PHP_INI_ENTRY("skywalking.grpc_tls_pem_root_certs", "", PHP_INI_ALL, OnUpdateString, grpc_tls_pem_root_certs, zend_skywalking_globals, skywalking_globals)
	STD_PHP_INI_ENTRY("skywalking.grpc_tls_pem_private_key", "", PHP_INI_ALL, OnUpdateString, grpc_tls_pem_private_key, zend_skywalking_globals, skywalking_globals)
	STD_PHP_INI_ENTRY("skywalking.grpc_tls_pem_cert_chain", "", PHP_INI_ALL, OnUpdateString, grpc_tls_pem_cert_chain, zend_skywalking_globals, skywalking_globals)

	STD_PHP_INI_BOOLEAN("skywalking.log_enable", "0", PHP_INI_ALL, OnUpdateBool, log_enable, zend_skywalking_globals, skywalking_globals)
	STD_PHP_INI_ENTRY("skywalking.log_path", "/tmp/skywalking-php.log", PHP_INI_ALL, OnUpdateString, log_path, zend_skywalking_globals, skywalking_globals)

    STD_PHP_INI_BOOLEAN("skywalking.error_handler_enable", "0", PHP_INI_ALL, OnUpdateBool, error_handler_enable, zend_skywalking_globals, skywalking_globals)

    STD_PHP_INI_ENTRY("skywalking.mq_max_message_length", "20480", PHP_INI_ALL, OnUpdateLong, mq_max_message_length, zend_skywalking_globals, skywalking_globals)

PHP_INI_END()

static void php_skywalking_init_globals(zend_skywalking_globals *skywalking_globals) {
    skywalking_globals->app_code = nullptr;
    skywalking_globals->enable = 0;
    skywalking_globals->version = 0;
    skywalking_globals->authentication = nullptr;

    // tls
    skywalking_globals->grpc = nullptr;
    skywalking_globals->grpc_tls_enable = 0;
    skywalking_globals->grpc_tls_pem_root_certs = nullptr;
    skywalking_globals->grpc_tls_pem_private_key = nullptr;
    skywalking_globals->grpc_tls_pem_cert_chain = nullptr;

    // log
    skywalking_globals->log_enable = 0;
    skywalking_globals->log_path = nullptr;

    // php error log
    skywalking_globals->error_handler_enable = 0;

    // message queue
    skywalking_globals->mq_max_message_length = 0;
}

PHP_FUNCTION (skywalking_trace_id) {
    auto *segment = sky_get_segment(execute_data, -1);
    if (SKYWALKING_G(enable) && segment != nullptr) {
        std::string trace_id = segment->getTraceId();
        RETURN_STRING(trace_id.c_str());
    } else {
        RETURN_STRING("");
    }
}

/* {{{ proto void skywalking_log(string key, string log [, bool is_error]) */
PHP_FUNCTION(skywalking_log)
{
    zend_string *name;
    zend_string *key;
    zend_string *value;
    zend_bool   is_error = 0;

    ZEND_PARSE_PARAMETERS_START(3, 4)
        Z_PARAM_STR(name)
        Z_PARAM_STR(key)
        Z_PARAM_STR(value)
        Z_PARAM_OPTIONAL
        Z_PARAM_BOOL(is_error)
    ZEND_PARSE_PARAMETERS_END();

    auto *segment = sky_get_segment(execute_data, -1);
    if (!SKYWALKING_G(enable) || segment == nullptr) {
        return;
    }

    if (ZSTR_LEN(name) > 0 && ZSTR_LEN(key) > 0 && ZSTR_LEN(value) > 0) {
        auto span = segment->findOrCreateSpan(name->val, SkySpanType::Local, SkySpanLayer::Unknown, 0);
        span->addLog(key->val, value->val);
        if (is_error) {
            span->setIsError(true);
        }
        span->setEndTIme();
    }
}

/* {{{ proto void skywalking_tag(string key, string value) */
PHP_FUNCTION(skywalking_tag)
{
    zend_string *name;
    zend_string *key;
    zend_string *value;

    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_STR(name)
        Z_PARAM_STR(key)
        Z_PARAM_STR(value)
    ZEND_PARSE_PARAMETERS_END();

    auto *segment = sky_get_segment(execute_data, -1);
    if (!SKYWALKING_G(enable) || segment == nullptr) {
        return;
    }

    if (ZSTR_LEN(name) > 0 && ZSTR_LEN(key) > 0 && ZSTR_LEN(value) > 0) {
        auto span = segment->findOrCreateSpan(name->val, SkySpanType::Local, SkySpanLayer::Unknown, 0);
        span->addTag(key->val, value->val);
        span->setEndTIme();
    }
}

PHP_MINIT_FUNCTION (skywalking) {
	ZEND_INIT_MODULE_GLOBALS(skywalking, php_skywalking_init_globals, NULL);
	REGISTER_INI_ENTRIES();

	if (SKYWALKING_G(enable)) {

        int protection = PROT_READ | PROT_WRITE;
        int visibility = MAP_SHARED | MAP_ANONYMOUS;

        s_info = (struct service_info *) mmap(nullptr, sizeof(struct service_info), protection, visibility, -1, 0);
        sky_module_init();
	}

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION (skywalking) {
    UNREGISTER_INI_ENTRIES();

    if (SKYWALKING_G(enable)) {
        sky_module_cleanup();
    }

    return SUCCESS;
}

PHP_RINIT_FUNCTION(skywalking)
{
#if defined(COMPILE_DL_SKYWALKING) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
    if (SKYWALKING_G(enable)) {
        if (strcasecmp("fpm-fcgi", sapi_module.name) == 0) {
            if (strlen(s_info->service_instance) == 0) {
                return SUCCESS;
            }

            sky_request_init(nullptr, 0);
        }
    }
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(skywalking)
{
	if (SKYWALKING_G(enable)) {
        if (strcasecmp("fpm-fcgi", sapi_module.name) == 0) {
            if (SKYWALKING_G(segment) == nullptr) {
                return SUCCESS;
            }

            sky_request_flush(nullptr, 0);
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_skywalking_trace_id, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_skywalking_log, 0, 0, 4)
    ZEND_ARG_INFO(0, name)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
    ZEND_ARG_INFO(0, is_error)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_skywalking_tag, 0, 0, 3)
    ZEND_ARG_INFO(0, name)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

const zend_function_entry skywalking_functions[] = {
        PHP_FE (skywalking_trace_id, arginfo_skywalking_trace_id)
        PHP_FE (skywalking_log,      arginfo_skywalking_log)
        PHP_FE (skywalking_tag,      arginfo_skywalking_tag)
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
