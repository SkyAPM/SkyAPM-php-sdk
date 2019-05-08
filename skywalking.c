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

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <sys/time.h>


#include "main/SAPI.h" /* for sapi_module */
#include "zend_smart_str.h" /* for smart_str */
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "src/components.h"
#include "php_skywalking.h"
#include "ext/standard/url.h" /* for php_url */
#include "ext/standard/php_var.h"

#include "ext/standard/basic_functions.h"
#include "ext/standard/php_math.h"
#include <string.h>
#include "ext/json/php_json.h"
#include "ext/date/php_date.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <dirent.h>


extern int applicationCodeRegister(char *grpc_server, char *code);

extern int registerInstance(char *grpc_server, int appId, char *uuid, long registertime, char *osname, char *hostname,
                            int processno, char *ipv4s);
extern char *uuid();

/* If you declare any globals in php_skywalking.h uncomment this:
*/
ZEND_DECLARE_MODULE_GLOBALS(skywalking)

/* True global resources - no need for thread safety here */
static int le_skywalking;
static int application_instance = 0;
static int application_id = 0;
static int sky_close = 0;
static int sky_increment_id = 0;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini*/
PHP_INI_BEGIN()
	STD_PHP_INI_BOOLEAN("skywalking.enable",   	"0", PHP_INI_ALL, OnUpdateBool, enable, zend_skywalking_globals, skywalking_globals)
	STD_PHP_INI_ENTRY("skywalking.version",   	"5", PHP_INI_ALL, OnUpdateLong, version, zend_skywalking_globals, skywalking_globals)
	STD_PHP_INI_ENTRY("skywalking.app_code", "", PHP_INI_ALL, OnUpdateString, app_code, zend_skywalking_globals, skywalking_globals)
    STD_PHP_INI_ENTRY("skywalking.log_path", "/tmp", PHP_INI_ALL, OnUpdateString, log_path, zend_skywalking_globals, skywalking_globals)
    STD_PHP_INI_ENTRY("skywalking.grpc", "127.0.0.1:11800", PHP_INI_ALL, OnUpdateString, grpc, zend_skywalking_globals, skywalking_globals)
PHP_INI_END()

/* }}} */



/* {{{ skywalking_functions[]
 *
 * Every user visible function must have an entry in skywalking_functions[].
 */
const zend_function_entry skywalking_functions[] = {
		/* For testing, remove later. */
	PHP_FE_END	/* Must be the last line in skywalking_functions[] */
};
/* }}} */



const zend_function_entry class_skywalking[] = {
	PHP_FE_END
};




/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/

void sky_curl_exec_handler(INTERNAL_FUNCTION_PARAMETERS)
{
	zval		*zid;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zid) == FAILURE) {
		return;
	}

	int is_send = 1;

    zval function_name,curlInfo;
    zval params[1];
    ZVAL_COPY(&params[0], zid);
    ZVAL_STRING(&function_name,  "curl_getinfo");
    call_user_function(CG(function_table), NULL, &function_name, &curlInfo, 1, params);
    zval_dtor(&function_name);
    zval_dtor(&params[0]);

    zval *z_url = zend_hash_str_find(Z_ARRVAL(curlInfo),  ZEND_STRL("url"));
    char *url_str = Z_STRVAL_P(z_url);
    if(strlen(url_str) <= 0) {
        zval_dtor(&curlInfo);
        is_send = 0;
    }
    php_url *url_info = NULL;
    if(is_send == 1) {
        url_info = php_url_parse(url_str);
        if(url_info->scheme == NULL || url_info->host == NULL) {
            zval_dtor(&curlInfo);
            php_url_free(url_info);
            is_send = 0;
        }
    }

    char *sw3 = NULL;
    zval *spans = NULL;
    zval *last_span = NULL;
    zval *span_id = NULL;
    char *peer = NULL;
    ssize_t operation_name_l = 0;
    char *operation_name = NULL;
    ssize_t full_url_l = 0;
    char *full_url = NULL;
    if (is_send == 1) {
        int peer_port = 0;
        if (url_info->port) {
            peer_port = url_info->port;
        } else {
            if (strcasecmp("http", url_info->scheme) == 0) {
                peer_port = 80;
            } else {
                peer_port = 443;
            }
        }

        peer = (char *) emalloc(strlen(url_info->scheme) + 3 + strlen(url_info->host) + 7);
        bzero(peer, strlen(url_info->scheme) + 3 + strlen(url_info->host) + 7);
        sprintf(peer, "%s://%s:%d", url_info->scheme, url_info->host, peer_port);

        if (url_info->query) {
            if (url_info->path == NULL) {
                operation_name_l = snprintf(NULL, 0, "%s", "/");
                operation_name = (char *) emalloc(operation_name_l + 1);
                bzero(operation_name, operation_name_l + 1);
                sprintf(operation_name, "%s", "/");

                full_url_l = snprintf(NULL, 0, "%s?%s", "/", url_info->query);
                full_url = (char *) emalloc(full_url_l + 1);
                bzero(full_url, full_url_l + 1);
                sprintf(full_url, "%s?%s", "/", url_info->query);
            } else {
                operation_name_l = snprintf(NULL, 0, "%s", url_info->path);
                operation_name = (char *) emalloc(operation_name_l + 1);
                bzero(operation_name, operation_name_l + 1);
                sprintf(operation_name, "%s", url_info->path);

                full_url_l = snprintf(NULL, 0, "%s?%s", url_info->path, url_info->query);
                full_url = (char *) emalloc(full_url_l + 1);
                bzero(full_url, full_url_l + 1);
                sprintf(full_url, "%s?%s", url_info->path, url_info->query);
            }
        } else {
            if (url_info->path == NULL) {
                operation_name_l = snprintf(NULL, 0, "%s", "/");
                operation_name = (char *) emalloc(operation_name_l + 1);
                bzero(operation_name, operation_name_l + 1);
                sprintf(operation_name, "%s", "/");

                full_url_l = snprintf(NULL, 0, "%s", "/");
                full_url = (char *) emalloc(full_url_l + 1);
                bzero(full_url, full_url_l + 1);
                sprintf(full_url, "%s", "/");
            } else {
                operation_name_l = snprintf(NULL, 0, "%s", url_info->path);
                operation_name = (char *) emalloc(operation_name_l + 1);
                bzero(operation_name, operation_name_l + 1);
                sprintf(operation_name, "%s", url_info->path);

                full_url_l = snprintf(NULL, 0, "%s", url_info->path);
                full_url = (char *) emalloc(full_url_l + 1);
                bzero(full_url, full_url_l + 1);
                sprintf(full_url, "%s", url_info->path);
            }
        }

        spans = get_spans();
        last_span = zend_hash_index_find(Z_ARRVAL_P(spans), zend_hash_num_elements(Z_ARRVAL_P(spans)) - 1);
        span_id = zend_hash_str_find(Z_ARRVAL_P(last_span), "spanId", sizeof("spanId") - 1);
        sw3 = generate_sw3(Z_LVAL_P(span_id) + 1, peer, operation_name);
    }


    if (sw3 != NULL) {
        zval *option = NULL;
        int is_init = 0;
        option = zend_hash_index_find(Z_ARRVAL_P(&SKYWALKING_G(curl_header)), Z_RES_HANDLE_P(zid));

        if(option == NULL) {
            option = emalloc(sizeof(zval));
            bzero(option, sizeof(zval));
            array_init(option);
            is_init = 1;
        }

        add_next_index_string(option, sw3);
        add_index_bool(&SKYWALKING_G(curl_header_send), (zend_ulong)Z_RES_HANDLE_P(zid), IS_TRUE);

        zval func;
        zval argv[3];
        zval ret;
        ZVAL_STRING(&func, "curl_setopt");

        ZVAL_COPY(&argv[0], zid);
        ZVAL_LONG(&argv[1], CURLOPT_HTTPHEADER);
        ZVAL_COPY(&argv[2], option);
        call_user_function(CG(function_table), NULL, &func, &ret, 3, argv);
        zval_dtor(&ret);
        zval_dtor(&func);
        if(is_init == 1) {
            zval_ptr_dtor(option);
            efree(option);
        }
        zval_dtor(&argv[0]);
        zval_dtor(&argv[1]);
        zval_dtor(&argv[2]);
        efree(sw3);
    }

    zval temp;
    char *l_millisecond;
    long millisecond;
    if(is_send == 1) {

        array_init(&temp);

        add_assoc_long(&temp, "spanId", Z_LVAL_P(span_id) + 1);
        add_assoc_long(&temp, "parentSpanId", 0);
        l_millisecond = get_millisecond();
        millisecond = zend_atol(l_millisecond, strlen(l_millisecond));
        efree(l_millisecond);
        add_assoc_long(&temp, "startTime", millisecond);
        add_assoc_long(&temp, "spanType", 1);
        add_assoc_long(&temp, "spanLayer", 3);
        add_assoc_long(&temp, "componentId", COMPONENT_HTTPCLIENT);
    }


	orig_curl_exec(INTERNAL_FUNCTION_PARAM_PASSTHRU);

    if (is_send == 1) {
        zval function_name_1, curlInfo_1;
        zval params_1[1];
        ZVAL_COPY(&params_1[0], zid);
        ZVAL_STRING(&function_name_1, "curl_getinfo");
        call_user_function(CG(function_table), NULL, &function_name_1, &curlInfo_1, 1, params_1);
        zval_dtor(&params_1[0]);
        zval_dtor(&function_name_1);

        zval *z_http_code;
        l_millisecond = get_millisecond();
        millisecond = zend_atol(l_millisecond, strlen(l_millisecond));
        efree(l_millisecond);

        z_http_code = zend_hash_str_find(Z_ARRVAL(curlInfo_1), ZEND_STRL("http_code"));

        add_assoc_long(&temp, "endTime", millisecond);


        add_assoc_string(&temp, "operationName", operation_name);
        add_assoc_string(&temp, "peer", peer);
        zval tags;
        array_init(&tags);
        add_assoc_string(&tags, "url", full_url);

        add_assoc_zval(&temp, "tags", &tags);

        efree(peer);
        efree(operation_name);
        efree(full_url);

        php_url_free(url_info);

        if (Z_LVAL_P(z_http_code) != 200) {
            add_assoc_long(&temp, "isError", 1);
        } else {
            add_assoc_long(&temp, "isError", 0);
        }
        zval _refs;
        array_init(&_refs);
        add_assoc_zval(&temp, "refs", &_refs);
        zend_hash_next_index_insert(Z_ARRVAL_P(spans), &temp);
        zval_dtor(&curlInfo_1);
        zval_dtor(&curlInfo);
    }
}

void sky_curl_setopt_handler(INTERNAL_FUNCTION_PARAMETERS) {
    zval *zid, *zvalue;
    zend_long options;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rlz", &zid, &options, &zvalue) == FAILURE) {
        return;
    }

    zval *is_send = zend_hash_index_find(Z_ARRVAL_P(&SKYWALKING_G(curl_header_send)), Z_RES_HANDLE_P(zid));
    //
    if (is_send != NULL &&  CURLOPT_HTTPHEADER == options && Z_TYPE_P(is_send) == IS_TRUE) {
        add_index_bool(&SKYWALKING_G(curl_header_send), Z_RES_HANDLE_P(zid), IS_FALSE);
        orig_curl_setopt(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    } else {
        if (CURLOPT_HTTPHEADER == options && Z_TYPE_P(zvalue) == IS_ARRAY) {
            zval copy_header;
            ZVAL_DUP(&copy_header, zvalue);
            add_index_zval(&SKYWALKING_G(curl_header), Z_RES_HANDLE_P(zid), &copy_header);
        } else {
            orig_curl_setopt(INTERNAL_FUNCTION_PARAM_PASSTHRU);
        }
    }
}

void sky_curl_setopt_array_handler(INTERNAL_FUNCTION_PARAMETERS) {
    zval *zid, *arr, *entry;
    zend_ulong option;
    zend_string *string_key;

    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_RESOURCE(zid)
            Z_PARAM_ARRAY(arr)
    ZEND_PARSE_PARAMETERS_END();

    zval *http_header = zend_hash_index_find(Z_ARRVAL_P(arr), CURLOPT_HTTPHEADER);

    if (http_header != NULL) {
        zval copy_header;
        ZVAL_DUP(&copy_header, http_header);
        add_index_zval(&SKYWALKING_G(curl_header), Z_RES_HANDLE_P(zid), &copy_header);
    }

    orig_curl_setopt_array(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void sky_curl_close_handler(INTERNAL_FUNCTION_PARAMETERS) {
    zval *zid;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zid) == FAILURE) {
        return;
    }

    zval *http_header = zend_hash_index_find(Z_ARRVAL_P(&SKYWALKING_G(curl_header)), Z_RES_HANDLE_P(zid));
    if (http_header != NULL) {
        zend_hash_index_del(Z_ARRVAL_P(&SKYWALKING_G(curl_header)), Z_RES_HANDLE_P(zid));
    }

    orig_curl_close(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* {{{ php_skywalking_init_globals
 */
/* Uncomment this function if you have INI entries*/
static void php_skywalking_init_globals(zend_skywalking_globals *skywalking_globals)
{
	skywalking_globals->app_code = NULL;
	skywalking_globals->log_path = NULL;
	skywalking_globals->enable = 0;
	skywalking_globals->version = 5;
	skywalking_globals->grpc = NULL;
}



static char *sky_json_encode(zval *parameter){

	smart_str buf = {0};
	zend_long options = 64;
#if PHP_VERSION_ID >= 71000
	return_code = php_json_encode(&buf, parameter, (int)options);
	if (return_code != SUCCESS) {
		smart_str_free(&buf);
		return NULL;
	}
#else
	php_json_encode(&buf, parameter, (int)options);
#endif
	smart_str_0(&buf);
	char *bufs = ZSTR_VAL(buf.s);
	smart_str_free(&buf);
	return bufs;
}


static void write_log(char *text) {
    if (application_instance != -100000) {
        char *log_path;
        char logFilename[100];
        char message[strlen(text) + 1];
        log_path = SKY_G(log_path);

        zend_string *_log_path, *_log_path_lower;
        _log_path = zend_string_init(log_path, strlen(log_path), 0);
        _log_path_lower = php_string_tolower(_log_path);

        bzero(logFilename, 100);
        sprintf(logFilename, "%s/skywalking.%d-%d.log", ZSTR_VAL(_log_path_lower), get_second(), getpid());

        zend_string_release(_log_path);
        zend_string_release(_log_path_lower);
        bzero(message, strlen(text));
        sprintf(message, "%s\n", text);
        _php_error_log_ex(3, message, strlen(message), logFilename, NULL);
    }

}


static char *generate_sw3(zend_long span_id, char *peer_host, char *operation_name) {

    zval *traceId = zend_hash_str_find(Z_ARRVAL(SKYWALKING_G(context)), "currentTraceId", sizeof("currentTraceId") - 1);
    zval *entryApplicationInstance = zend_hash_str_find(Z_ARRVAL(SKYWALKING_G(context)), "entryApplicationInstance",
                                                        sizeof("entryApplicationInstance") - 1);
    zval *entryOperationName = zend_hash_str_find(Z_ARRVAL(SKYWALKING_G(context)), "entryOperationName",
                                                  sizeof("entryOperationName") - 1);
    zval *distributedTraceId = zend_hash_str_find(Z_ARRVAL(SKYWALKING_G(context)), "distributedTraceId",
                                                  sizeof("distributedTraceId") - 1);
    ssize_t sw3_l = 0;
    sw3_l = snprintf(NULL, 0, "sw3: %s|%d|%d|%d|#%s|#%s|#%s|%s", Z_STRVAL_P(traceId), span_id,
                     application_instance, Z_LVAL_P(entryApplicationInstance), peer_host,
                     Z_STRVAL_P(entryOperationName), operation_name, Z_STRVAL_P(distributedTraceId));
    char *sw3 = (char*)emalloc(sw3_l + 1);
    bzero(sw3, sw3_l + 1);
    snprintf(sw3, sw3_l + 1, "sw3: %s|%d|%d|%d|#%s|#%s|#%s|%s", Z_STRVAL_P(traceId), span_id,
             application_instance, Z_LVAL_P(entryApplicationInstance), peer_host,
             Z_STRVAL_P(entryOperationName), operation_name, Z_STRVAL_P(distributedTraceId));
    return sw3;
}

static zend_string *trim_sharp(zval *tmp) {
    return php_trim(Z_STR_P(tmp), "#", sizeof("#") - 1, 1);
}

static void generate_context() {
    int sys_pid = getpid();
    long second = get_second();
    second = second * 10000 + sky_increment_id;
    char *makeTraceId;
    makeTraceId = (char *) emalloc(sizeof(char) * 180);

    bzero(makeTraceId, sizeof(char) * 180);

    sprintf(makeTraceId, "%d.%d.%ld", application_instance, sys_pid, second);

    add_assoc_string(&SKYWALKING_G(context), "currentTraceId", makeTraceId);
    add_assoc_long(&SKYWALKING_G(context), "isChild", 0);

    // parent
    zval *carrier = NULL;
    zval *sw3;

    zend_bool jit_initialization = PG(auto_globals_jit);

    if (jit_initialization) {
        zend_string *server_str = zend_string_init("_SERVER", sizeof("_SERVER") - 1, 0);
        zend_is_auto_global(server_str);
        zend_string_release(server_str);
    }
    carrier = zend_hash_str_find(&EG(symbol_table), ZEND_STRL("_SERVER"));
    sw3 = zend_hash_str_find(Z_ARRVAL_P(carrier), "HTTP_SW3", sizeof("HTTP_SW3") - 1);

    if (sw3 != NULL && Z_TYPE_P(sw3) == IS_STRING && Z_STRLEN_P(sw3) > 10) {
        add_assoc_string(&SKYWALKING_G(context), "sw3", Z_STRVAL_P(sw3));

        zval temp;
        array_init(&temp);

        php_explode(zend_string_init(ZEND_STRL("|"), 0), Z_STR_P(sw3), &temp, 10);

        if(zend_array_count(Z_ARRVAL_P(&temp)) >= 8) {
            zval *sw3_0 = zend_hash_index_find(Z_ARRVAL(temp), 0);
            zval *sw3_1 = zend_hash_index_find(Z_ARRVAL(temp), 1);
            zval *sw3_2 = zend_hash_index_find(Z_ARRVAL(temp), 2);
            zval *sw3_3 = zend_hash_index_find(Z_ARRVAL(temp), 3);
            zval *sw3_4 = zend_hash_index_find(Z_ARRVAL(temp), 4);
            zval *sw3_5 = zend_hash_index_find(Z_ARRVAL(temp), 5);
            zval *sw3_6 = zend_hash_index_find(Z_ARRVAL(temp), 6);
            zval *sw3_7 = zend_hash_index_find(Z_ARRVAL(temp), 7);

            zval child;
            array_init(&child);
            ZVAL_LONG(&child, 1);
            zend_hash_str_update(Z_ARRVAL_P(&SKYWALKING_G(context)), "isChild", sizeof("isChild") - 1, &child);

            add_assoc_string(&SKYWALKING_G(context), "parentTraceSegmentId", Z_STRVAL_P(sw3_0));
            add_assoc_long(&SKYWALKING_G(context), "parentSpanId", zend_atol(Z_STRVAL_P(sw3_1), sizeof(Z_STRVAL_P(sw3_1)) - 1));
            add_assoc_long(&SKYWALKING_G(context), "parentApplicationInstance", zend_atol(Z_STRVAL_P(sw3_2), sizeof(Z_STRVAL_P(sw3_2)) - 1));
            add_assoc_long(&SKYWALKING_G(context), "entryApplicationInstance", zend_atol(Z_STRVAL_P(sw3_3), sizeof(Z_STRVAL_P(sw3_3)) - 1));
            add_assoc_str(&SKYWALKING_G(context), "networkAddress", trim_sharp(sw3_4));
            add_assoc_str(&SKYWALKING_G(context), "entryOperationName", trim_sharp(sw3_5));
            add_assoc_str(&SKYWALKING_G(context), "parentOperationName", trim_sharp(sw3_6));
            add_assoc_string(&SKYWALKING_G(context), "distributedTraceId", Z_STRVAL_P(sw3_7));
        }
    } else {
        add_assoc_long(&SKYWALKING_G(context), "parentApplicationInstance", application_instance);
        add_assoc_long(&SKYWALKING_G(context), "entryApplicationInstance", application_instance);
        add_assoc_string(&SKYWALKING_G(context), "entryOperationName", get_page_request_uri());
        add_assoc_string(&SKYWALKING_G(context), "distributedTraceId", makeTraceId);
    }
    efree(makeTraceId);
}


static long get_second() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec;
}

static char *get_millisecond(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	char *buffer;
	buffer = (char *)emalloc(sizeof(char)*20);
	bzero(buffer, 20);
	long millisecond;
	millisecond = tv.tv_sec*1000 + tv.tv_usec/1000;
	sprintf(buffer, "%ld",  millisecond);

	return buffer;
}

static char *get_page_request_peer() {
    zval *carrier = NULL;
    zval *request_host = NULL;
    zval *request_port = NULL;

    char *peer = NULL;
    size_t peer_l = 0;

    zend_bool jit_initialization = PG(auto_globals_jit);

    if (jit_initialization) {
        zend_string *server_str = zend_string_init("_SERVER", sizeof("_SERVER") - 1, 0);
        zend_is_auto_global(server_str);
        zend_string_release(server_str);
    }
    carrier = zend_hash_str_find(&EG(symbol_table), ZEND_STRL("_SERVER"));

    request_host = zend_hash_str_find(Z_ARRVAL_P(carrier), "SERVER_ADDR", sizeof("SERVER_ADDR") - 1);
    request_port = zend_hash_str_find(Z_ARRVAL_P(carrier), "SERVER_PORT", sizeof("SERVER_PORT") - 1);

    if (request_host != NULL && request_port != NULL) {
        peer_l = snprintf(NULL, 0, "%s:%s", Z_STRVAL_P(request_host), Z_STRVAL_P(request_port));
        peer = emalloc(peer_l + 1);
        snprintf(peer, peer_l + 1, "%s:%s", Z_STRVAL_P(request_host), Z_STRVAL_P(request_port));
    }

    return peer;
}

static char *get_page_request_uri() {
    zval *carrier = NULL;
    zval *request_uri;

    smart_str uri = {0};

    if (strcasecmp("cli", sapi_module.name) == 0) {
        smart_str_appendl(&uri, "cli", strlen("cli"));
    } else {
        zend_bool jit_initialization = PG(auto_globals_jit);

        if (jit_initialization) {
            zend_string *server_str = zend_string_init("_SERVER", sizeof("_SERVER") - 1, 0);
            zend_is_auto_global(server_str);
            zend_string_release(server_str);
        }
        carrier = zend_hash_str_find(&EG(symbol_table), ZEND_STRL("_SERVER"));

        request_uri = zend_hash_str_find(Z_ARRVAL_P(carrier), "REQUEST_URI", sizeof("REQUEST_URI") - 1);
        smart_str_appendl(&uri, Z_STRVAL_P(request_uri), strlen(Z_STRVAL_P(request_uri)));
    }

    smart_str_0(&uri);
    char *uris = ZSTR_VAL(uri.s);
    smart_str_free(&uri);
    return uris;
}


/**
 * ip
 * 
 * @since 2017年11月23日
 * @copyright
 * @return return_type
//  */
static char* _get_current_machine_ip(){


	char *ip;
	zval *carrier = NULL;
	ip  = (char *)emalloc(sizeof(char)*100);

	bzero(ip, 100);

	carrier = &PG(http_globals)[TRACK_VARS_SERVER];

	if (strcasecmp("cli", sapi_module.name) == 0){
		strcpy(ip, "127.0.0.1");
	}else{
		char hname[128];
    	struct hostent *hent;
     	gethostname(hname, sizeof(hname));
		hent = gethostbyname(hname);
		ip = inet_ntoa(*(struct in_addr*)(hent->h_addr_list[0]));
	}


    return ip;
}

static void request_init() {

    array_init(&SKYWALKING_G(curl_header));
    array_init(&SKYWALKING_G(curl_header_send));
    array_init(&SKYWALKING_G(context));
	array_init(&SKYWALKING_G(UpstreamSegment));

    generate_context();

    add_assoc_long(&SKYWALKING_G(UpstreamSegment), "application_instance", application_instance);
    add_assoc_long(&SKYWALKING_G(UpstreamSegment), "pid", getppid());
    add_assoc_long(&SKYWALKING_G(UpstreamSegment), "application_id", application_id);
    add_assoc_long(&SKYWALKING_G(UpstreamSegment), "version", SKYWALKING_G(version));
	SKY_ADD_ASSOC_ZVAL(&SKYWALKING_G(UpstreamSegment), "segment");
	SKY_ADD_ASSOC_ZVAL(&SKYWALKING_G(UpstreamSegment), "globalTraceIds");

	zval *traceId = zend_hash_str_find(Z_ARRVAL(SKYWALKING_G(context)), "currentTraceId", sizeof("currentTraceId") - 1);

	zval traceSegmentObject;
	zval spans;
	array_init(&spans);
	array_init(&traceSegmentObject);
	add_assoc_string(&traceSegmentObject, "traceSegmentId", Z_STRVAL_P(traceId));
	add_assoc_long(&traceSegmentObject, "isSizeLimited", 0);

	zval temp;
    char *peer = NULL;
    char *uri = get_page_request_uri();
    char *path = (char*)emalloc(sizeof(char) * strlen(uri) - 1);

    int i;
    for(i = 0; i < strlen(uri); i++) {
        if (uri[i] == '?') {
            break;
        }
        path[i] = uri[i];
    }

    path[i] = '\0';

	array_init(&temp);
	peer = get_page_request_peer();

    zval tags;
    array_init(&tags);
    add_assoc_string(&tags, "url", (uri == NULL) ? "" : uri);

    add_assoc_zval(&temp, "tags", &tags);

    add_assoc_long(&temp, "spanId", 0);
    add_assoc_long(&temp, "parentSpanId", -1);
    char *l_millisecond = get_millisecond();
    long millisecond = zend_atol(l_millisecond, strlen(l_millisecond));
    efree(l_millisecond);
    add_assoc_long(&temp, "startTime", millisecond);
    add_assoc_string(&temp, "operationName", path);
    efree(path);
    add_assoc_string(&temp, "peer", (peer == NULL) ? "" : peer);
    add_assoc_long(&temp, "spanType", 0);
    add_assoc_long(&temp, "spanLayer", 3);
    add_assoc_long(&temp, "componentId", COMPONENT_HTTPCLIENT);

    zval *isChild = zend_hash_str_find(Z_ARRVAL_P(&SKYWALKING_G(context)), "isChild", sizeof("isChild") - 1);
    // refs
    zval refs;
    array_init(&refs);

    zval globalTraceIds;
    array_init(&globalTraceIds);
    zval tmpGlobalTraceIds;

    if(Z_LVAL_P(isChild) == 1) {
        zval ref;
        array_init(&ref);
        zval *parentTraceSegmentId = zend_hash_str_find(Z_ARRVAL(SKYWALKING_G(context)), "parentTraceSegmentId", sizeof("parentTraceSegmentId") - 1);
        zval *parentSpanId = zend_hash_str_find(Z_ARRVAL(SKYWALKING_G(context)), "parentSpanId", sizeof("parentSpanId") - 1);
        zval *parentApplicationInstance = zend_hash_str_find(Z_ARRVAL(SKYWALKING_G(context)), "parentApplicationInstance", sizeof("parentApplicationInstance") - 1);
        zval *entryApplicationInstance = zend_hash_str_find(Z_ARRVAL(SKYWALKING_G(context)), "entryApplicationInstance", sizeof("entryApplicationInstance") - 1);
        zval *entryOperationName = zend_hash_str_find(Z_ARRVAL(SKYWALKING_G(context)), "entryOperationName", sizeof("entryOperationName") - 1);
        zval *networkAddress = zend_hash_str_find(Z_ARRVAL(SKYWALKING_G(context)), "networkAddress", sizeof("networkAddress") - 1);
        zval *parentOperationName = zend_hash_str_find(Z_ARRVAL(SKYWALKING_G(context)), "parentOperationName", sizeof("parentOperationName") - 1);
        zval *distributedTraceId = zend_hash_str_find(Z_ARRVAL(SKYWALKING_G(context)), "distributedTraceId", sizeof("distributedTraceId") - 1);
        add_assoc_long(&ref, "type", 0);
        add_assoc_string(&ref, "parentTraceSegmentId", Z_STRVAL_P(parentTraceSegmentId));
        add_assoc_long(&ref, "parentSpanId", Z_LVAL_P(parentSpanId));
        add_assoc_long(&ref, "parentApplicationInstanceId", Z_LVAL_P(parentApplicationInstance));
        add_assoc_string(&ref, "networkAddress", Z_STRVAL_P(networkAddress));
        add_assoc_long(&ref, "entryApplicationInstanceId", Z_LVAL_P(entryApplicationInstance));
        add_assoc_string(&ref, "entryServiceName", Z_STRVAL_P(entryOperationName));
        add_assoc_string(&ref, "parentServiceName", Z_STRVAL_P(parentOperationName));
        zend_hash_next_index_insert(Z_ARRVAL(refs), &ref);
        ZVAL_STRING(&tmpGlobalTraceIds, Z_STRVAL_P(distributedTraceId));
    } else {
        ZVAL_STRING(&tmpGlobalTraceIds, Z_STRVAL_P(traceId));

    }

    zend_hash_str_add(Z_ARRVAL(temp), "refs", sizeof("refs") - 1, &refs);
    zend_hash_next_index_insert(Z_ARRVAL(spans), &temp);

    add_assoc_zval(&traceSegmentObject, "spans", &spans);

    zend_hash_next_index_insert(Z_ARRVAL(globalTraceIds), &tmpGlobalTraceIds);

    zend_hash_str_update(Z_ARRVAL(SKYWALKING_G(UpstreamSegment)), "segment", sizeof("segment") - 1, &traceSegmentObject);
    zend_hash_str_update(Z_ARRVAL(SKYWALKING_G(UpstreamSegment)), "globalTraceIds", sizeof("globalTraceIds") - 1, &globalTraceIds);
}


static void sky_flush_all() {
	char *l_millisecond = get_millisecond();
	long millisecond = zend_atol(l_millisecond, strlen(l_millisecond));
	efree(l_millisecond);

	zval *span = get_first_span();

	add_assoc_long(span, "endTime", millisecond);
	if ((SG(sapi_headers).http_response_code >= 500)) {
		add_assoc_long(span, "isError", 1);
	} else {
		add_assoc_long(span, "isError", 0);
	}

	write_log(sky_json_encode(&SKYWALKING_G(UpstreamSegment)));
}

static zval *get_first_span() {
	zval *segment = zend_hash_str_find(Z_ARRVAL(SKYWALKING_G(UpstreamSegment)), "segment", sizeof("segment") - 1);
	zval *spans = zend_hash_str_find(Z_ARRVAL_P(segment), "spans", sizeof("spans") - 1);
	zval *span = zend_hash_index_find(Z_ARRVAL_P(spans), 0);
	return span;
}

static zval *get_spans() {
	zval *segment = zend_hash_str_find(Z_ARRVAL(SKYWALKING_G(UpstreamSegment)), "segment", sizeof("segment") - 1);
	zval *spans = zend_hash_str_find(Z_ARRVAL_P(segment), "spans", sizeof("spans") - 1);
	return spans;
}


static void module_init() {

    application_instance = -100000;
    application_id = -100000;

    int i = 0;

    do {
        application_id = applicationCodeRegister(SKYWALKING_G(grpc), SKYWALKING_G(app_code));

        if(application_id == -100000) {
            sleep(1);
        }

        i++;
    } while (application_id == -100000 && i <= 3);

    if (application_id == -100000) {
        sky_close = 1;
        return;
    }

    char *ipv4s = _get_current_machine_ip();

    char hostname[100] = {0};
    if (gethostname(hostname, sizeof(hostname)) < 0) {
        strcpy(hostname, "");
    }

    char *l_millisecond = get_millisecond();
    long millisecond = zend_atol(l_millisecond, strlen(l_millisecond));
    efree(l_millisecond);

    char *uid = uuid();
    i = 0;
    do {
        application_instance = registerInstance(SKYWALKING_G(grpc), application_id, uid, millisecond, SKY_OS_NAME,
                                                hostname, getpid(),
                                                ipv4s);
        if(application_instance == -100000) {
            sleep(1);
        }
        i++;
    } while (application_instance == -100000 && i <= 3);


    if (application_instance == -100000) {
        sky_close = 1;
        return;
    }
}


/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION (skywalking) {
	ZEND_INIT_MODULE_GLOBALS(skywalking, php_skywalking_init_globals, NULL);
	//data_register_hashtable();
	REGISTER_INI_ENTRIES();
	/* If you have INI entries, uncomment these lines
	*/
	if (SKYWALKING_G(enable)) {
        if (strcasecmp("cli", sapi_module.name) == 0) {
            sky_close = 1;
        } else {
            module_init();
        }

		if (sky_close == 1) {
			return SUCCESS;
		}
//		set_sampling_rate(SKY_G(global_sampling_rate));
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
	}

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(skywalking)
{
	UNREGISTER_INI_ENTRIES();
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION7
 */
PHP_RINIT_FUNCTION(skywalking)
{

#if defined(COMPILE_DL_SKYWALKING) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	if (SKYWALKING_G(enable)) {
        if (sky_close == 1) {
            return SUCCESS;
        }
		sky_increment_id++;
		if (sky_increment_id >= 9999) {
			sky_increment_id = 0;
		}
        request_init();
	}
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(skywalking)
{

	if(SKYWALKING_G(enable)){
        if (sky_close == 1) {
            return SUCCESS;
        }
		sky_flush_all();
        zval_dtor(&SKYWALKING_G(context));
        zval_dtor(&SKYWALKING_G(curl_header));
        zval_dtor(&SKYWALKING_G(curl_header_send));
        zval_dtor(&SKYWALKING_G(UpstreamSegment));
	}
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(skywalking)
{

	php_info_print_table_start();
    if (sky_close) {
        php_info_print_table_header(2, "skywalking Support", "disabled (register fail)");
    } else {
        php_info_print_table_header(2, "skywalking Support", "enabled");
    }
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_GINIT_FUNCTION(skywalking)
{
    memset(skywalking_globals, 0, sizeof(*skywalking_globals));
}
/* }}} */

zend_module_dep skywalking_deps[] = {
        ZEND_MOD_REQUIRED("json")
        {NULL, NULL, NULL}
};

/* {{{ skywalking_module_entry
 */
zend_module_entry skywalking_module_entry = {
	STANDARD_MODULE_HEADER,
	"skywalking",
	skywalking_functions,
	PHP_MINIT(skywalking),
	PHP_MSHUTDOWN(skywalking),
	PHP_RINIT(skywalking),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(skywalking),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(skywalking),
	PHP_SKYWALKING_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_SKYWALKING
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(skywalking)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
