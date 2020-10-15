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


#ifndef PHP_SKYWALKING_H
#define PHP_SKYWALKING_H

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
#include "zend_interfaces.h"
#include "ext/pdo/php_pdo_driver.h"

extern zend_module_entry skywalking_module_entry;
#define phpext_skywalking_ptr &skywalking_module_entry

#define SKY_DEBUG 0
#define PHP_SKYWALKING_VERSION "4.0.0"

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

    // tls
    zend_bool grpc_tls_enable;
    char *grpc_tls_pem_root_certs;
    char *grpc_tls_pem_private_key;
    char *grpc_tls_pem_cert_chain;
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
