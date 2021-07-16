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


#ifndef PHP_SKYWALKING_H
#define PHP_SKYWALKING_H

#include "thread"

#include "src/common.h"

SKY_BEGIN_EXTERN_C()

#include "php.h"
#include "php_ini.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "main/SAPI.h"
#include "zend_API.h"
#include <curl/curl.h>
#include "ext/standard/url.h"
#if PHP_VERSION_ID >= 80000
#include "ext/curl/php_curl.h"
#endif
#include "zend_interfaces.h"
#include "ext/pdo/php_pdo_driver.h"
#include "ext/standard/php_var.h"

#include "zend_smart_str.h"
#include "Zend/zend_smart_str.h"
#include "ext/json/php_json.h"
#include "ext/standard/php_smart_string.h"

extern zend_module_entry skywalking_module_entry;
#define phpext_skywalking_ptr &skywalking_module_entry

#define PHP_SKYWALKING_VERSION "4.1.2"


#define SKY_STRCMP(s1, s2) ((s1) != nullptr && strcmp(s1, s2) == 0)

#define SKY_IS_SWOOLE(func_name) (SKY_STRCMP(func_name, "{closure}"))
#define SKY_IS_HYPERF(class_name, func_name) (SKY_STRCMP(class_name, "Hyperf\\HttpServer\\Server") && SKY_STRCMP(func_name, "onRequest"))
#define SKY_IS_SWOFT(class_name, func_name) (SKY_STRCMP(class_name, "Swoft\\Http\\Server\\Swoole\\RequestListener") && SKY_STRCMP(func_name, "onRequest"))
#define SKY_IS_TARS(class_name, func_name) (SKY_STRCMP(class_name, "Tars\\core\\Server") && SKY_STRCMP(func_name, "onRequest"))
#define SKY_IS_LARAVELS(class_name, func_name) ((SKY_STRCMP(class_name, "Hhxsv\\LaravelS\\LaravelS") || SKY_STRCMP(class_name, "Hhxsv5\\LaravelS\\LaravelS")) && SKY_STRCMP(func_name, "onRequest"))
#define SKY_IS_SWOOLE_FRAMEWORK(class_name, func_name) SKY_IS_HYPERF(class_name, func_name) || SKY_IS_SWOFT(class_name, func_name) || SKY_IS_TARS(class_name, func_name) || SKY_IS_LARAVELS(class_name, func_name)

#if PHP_VERSION_ID < 80000
#define SKY_ZEND_CALL_METHOD(obj, fn, func, ret, param, arg1, arg2) zend_call_method(obj, Z_OBJCE_P(obj), fn, ZEND_STRL(func), ret, param, arg1, arg2);
#else
#define SKY_ZEND_CALL_METHOD(obj, fn, func, ret, param, arg1, arg2) zend_call_method(Z_OBJ_P(obj), Z_OBJCE_P(obj), fn, ZEND_STRL(func), ret, param, arg1, arg2);
#endif


#ifdef PHP_WIN32
#	define PHP_SKYWALKING_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_SKYWALKING_API __attribute__ ((visibility("default")))
#else
#	define PHP_SKYWALKING_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_FUNCTION (skywalking_trace_id);
PHP_FUNCTION (skywalking_log);
PHP_FUNCTION (skywalking_tag);
PHP_MINIT_FUNCTION (skywalking);
PHP_MSHUTDOWN_FUNCTION (skywalking);
PHP_RINIT_FUNCTION (skywalking);
PHP_RSHUTDOWN_FUNCTION (skywalking);
PHP_MINFO_FUNCTION (skywalking);


ZEND_BEGIN_MODULE_GLOBALS(skywalking)
    char *authentication;
    char *app_code;
    char *grpc;
    zend_bool enable;
    zval context;
    zval curl_header;
    int version;

    void *segment;
    zend_bool is_swoole;

    // tls
    zend_bool grpc_tls_enable;
    char *grpc_tls_pem_root_certs;
    char *grpc_tls_pem_private_key;
    char *grpc_tls_pem_cert_chain;

    // log
    zend_bool log_enable;
    char *log_path;

    // php error log
    zend_bool error_handler_enable;

    // message queue
    int mq_max_message_length;
ZEND_END_MODULE_GLOBALS(skywalking)

extern ZEND_DECLARE_MODULE_GLOBALS(skywalking);

#ifdef ZTS
#define SKYWALKING_G(v) TSRMG(skywalking_globals_id, zend_skywalking_globals *, v)
#else
#define SKYWALKING_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(skywalking, v)
#endif

#if defined(ZTS) && defined(COMPILE_DL_SKYWALKING)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

SKY_END_EXTERN_C()
#endif
