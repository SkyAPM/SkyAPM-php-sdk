/*
 * Copyright 2021 SkyAPM
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *
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
# include "config.h"
#endif

#include "php.h"
#include "SAPI.h"
#include "ext/standard/info.h"
#include "php_skywalking.h"

#include "sky_core_module.h"

#ifdef MYSQLI_USE_MYSQLND
#include "ext/mysqli/php_mysqli_structs.h"
#endif

ZEND_DECLARE_MODULE_GLOBALS(skywalking)

PHP_INI_BEGIN()
    STD_PHP_INI_BOOLEAN("skywalking.enable", "0", PHP_INI_ALL, OnUpdateBool, enable, zend_skywalking_globals, skywalking_globals)

    STD_PHP_INI_ENTRY("skywalking.service", "hello_skywalking", PHP_INI_ALL, OnUpdateString, service, zend_skywalking_globals, skywalking_globals)
    STD_PHP_INI_ENTRY("skywalking.service_instance", "", PHP_INI_ALL, OnUpdateString, service_instance, zend_skywalking_globals, skywalking_globals)

	STD_PHP_INI_ENTRY("skywalking.oap_version", "9.0.0", PHP_INI_ALL, OnUpdateString, oap_version, zend_skywalking_globals, skywalking_globals)
	STD_PHP_INI_ENTRY("skywalking.oap_cross_process_protocol", "3.0", PHP_INI_ALL, OnUpdateString, oap_cross_process_protocol, zend_skywalking_globals, skywalking_globals)
	STD_PHP_INI_ENTRY("skywalking.oap_authentication", "", PHP_INI_ALL, OnUpdateString, oap_authentication, zend_skywalking_globals, skywalking_globals)

	STD_PHP_INI_ENTRY("skywalking.grpc_address", "127.0.0.1:11800", PHP_INI_ALL, OnUpdateString, grpc_address, zend_skywalking_globals, skywalking_globals)
	STD_PHP_INI_BOOLEAN("skywalking.grpc_tls_enable", "0", PHP_INI_ALL, OnUpdateBool, grpc_tls_enable, zend_skywalking_globals, skywalking_globals)
	STD_PHP_INI_ENTRY("skywalking.grpc_tls_pem_root_certs", "", PHP_INI_ALL, OnUpdateString, grpc_tls_pem_root_certs, zend_skywalking_globals, skywalking_globals)
	STD_PHP_INI_ENTRY("skywalking.grpc_tls_pem_private_key", "", PHP_INI_ALL, OnUpdateString, grpc_tls_pem_private_key, zend_skywalking_globals, skywalking_globals)
	STD_PHP_INI_ENTRY("skywalking.grpc_tls_pem_cert_chain", "", PHP_INI_ALL, OnUpdateString, grpc_tls_pem_cert_chain, zend_skywalking_globals, skywalking_globals)

    STD_PHP_INI_ENTRY("skywalking.log_level", "disable", PHP_INI_ALL, OnUpdateString, log_level, zend_skywalking_globals, skywalking_globals)
	STD_PHP_INI_ENTRY("skywalking.log_path", "/tmp/skywalking-php.log", PHP_INI_ALL, OnUpdateString, log_path, zend_skywalking_globals, skywalking_globals)

    STD_PHP_INI_BOOLEAN("skywalking.curl_response_enable", "0", PHP_INI_ALL, OnUpdateBool, curl_response_enable, zend_skywalking_globals, skywalking_globals)

    STD_PHP_INI_BOOLEAN("skywalking.error_handler_enable", "0", PHP_INI_ALL, OnUpdateBool, error_handler_enable, zend_skywalking_globals, skywalking_globals)

    STD_PHP_INI_ENTRY("skywalking.mq_max_message_length", "20480", PHP_INI_ALL, OnUpdateLong, mq_max_message_length, zend_skywalking_globals, skywalking_globals)
    STD_PHP_INI_ENTRY("skywalking.mq_unique", "0", PHP_INI_ALL, OnUpdateBool, mq_unique, zend_skywalking_globals, skywalking_globals)

    STD_PHP_INI_ENTRY("skywalking.sample_n_per_3_secs", "-1", PHP_INI_ALL, OnUpdateLong, sample_n_per_3_secs, zend_skywalking_globals, skywalking_globals)

PHP_INI_END()

static void php_skywalking_init_globals(zend_skywalking_globals *skywalking_globals) {
    skywalking_globals->enable = 0;

    skywalking_globals->service = NULL;
    skywalking_globals->service_instance = NULL;
    skywalking_globals->real_service_instance = "";

    skywalking_globals->oap_version = "9.0.0";
    skywalking_globals->oap_cross_process_protocol = "3.0";
    skywalking_globals->oap_authentication = NULL;

    // tls
    skywalking_globals->grpc_address = NULL;
    skywalking_globals->grpc_tls_enable = 0;
    skywalking_globals->grpc_tls_pem_root_certs = NULL;
    skywalking_globals->grpc_tls_pem_private_key = NULL;
    skywalking_globals->grpc_tls_pem_cert_chain = NULL;

    // log
    skywalking_globals->log_level = "disable";
    skywalking_globals->log_path = NULL;

    skywalking_globals->curl_response_enable = 0;
    // php error log
    skywalking_globals->error_handler_enable = 0;

    // message queue
    skywalking_globals->mq_max_message_length = 0;

    // rate limit
    skywalking_globals->sample_n_per_3_secs = -1;
}

PHP_FUNCTION (skywalking_trace_id) {
//    auto *segment = sky_get_segment(execute_data, -1);
//    if (SKYWALKING_G(enable) && segment != NULL) {
//        std::string trace_id = segment->getTraceId();
//        RETURN_STRING(trace_id.c_str());
//    } else {
        RETURN_STRING("");
//    }
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

//    auto *segment = sky_get_segment(execute_data, -1);
//    if (!SKYWALKING_G(enable) || segment == NULL) {
//        return;
//    }
//
//    if (ZSTR_LEN(name) > 0 && ZSTR_LEN(key) > 0 && ZSTR_LEN(value) > 0) {
//        auto span = segment->findOrCreateSpan(name->val, SkyCoreSpanType::Local, SkyCoreSpanLayer::Unknown, 0);
//        span->addLog(key->val, value->val);
//        if (is_error) {
//            span->setIsError(true);
//        }
//        span->setEndTIme();
//    }
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

//    auto *segment = sky_get_segment(execute_data, -1);
//    if (!SKYWALKING_G(enable) || segment == nullptr) {
//        return;
//    }
//
//    if (ZSTR_LEN(name) > 0 && ZSTR_LEN(key) > 0 && ZSTR_LEN(value) > 0) {
//        auto span = segment->findOrCreateSpan(name->val, SkyCoreSpanType::Local, SkyCoreSpanLayer::Unknown, 0);
//        span->addTag(key->val, value->val);
//        span->setEndTIme();
//    }
}

PHP_MINIT_FUNCTION (skywalking) {
	ZEND_INIT_MODULE_GLOBALS(skywalking, php_skywalking_init_globals, NULL);
	REGISTER_INI_ENTRIES();

	if (SKYWALKING_G(enable)) {
        sky_core_module_init(INIT_FUNC_ARGS_PASSTHRU);
	}

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION (skywalking) {
    UNREGISTER_INI_ENTRIES();

    if (SKYWALKING_G(enable)) {
        sky_core_module_free();
    }

    return SUCCESS;
}

PHP_RINIT_FUNCTION(skywalking)
{
#if defined(COMPILE_DL_SKYWALKING) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
    if (SKYWALKING_G(enable)) {
        sky_core_request_init(NULL, 0);
    }
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION (skywalking) {
    if (SKYWALKING_G(enable)) {
        sky_core_request_free(NULL, 0);
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
