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

#include <uuid/uuid.h>

#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <dirent.h>


extern int applicationCodeRegister(char *grpc_server, char *code);

extern int registerInstance(char *grpc_server, int appId, long registertime, char *uuid, char *osname, char *hostname,
                            int processno, char *ipv4s);

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


	zval *spans = get_spans();
	zval *last_span = zend_hash_index_find(Z_ARRVAL_P(spans), zend_hash_num_elements(Z_ARRVAL_P(spans)) - 1);
	zval *span_id = zend_hash_str_find(Z_ARRVAL_P(last_span), "spanId", sizeof("spanId") - 1);

	zval temp;
	array_init(&temp);

	add_assoc_long(&temp, "spanId", Z_LVAL_P(span_id) + 1);
	add_assoc_long(&temp, "parentSpanId", 0);
	char *l_millisecond;
	l_millisecond = get_millisecond();
	long millisecond;
	millisecond = zend_atol(l_millisecond, strlen(l_millisecond));
	efree(l_millisecond);
	add_assoc_long(&temp, "startTime", millisecond);
	add_assoc_long(&temp, "spanType", 1);
	add_assoc_long(&temp, "spanLayer", 3);
	add_assoc_long(&temp, "componentId", COMPONENT_HTTPCLIENT);

    zval function_name,curlInfo;
    zval params[1];
    ZVAL_COPY(&params[0], zid);
    ZVAL_STRING(&function_name,  "curl_getinfo");
    call_user_function(CG(function_table), NULL, &function_name, &curlInfo, 1, params);
    zval_dtor(&function_name);
    zval_dtor(&params[0]);

    zval *z_url = zend_hash_str_find(Z_ARRVAL(curlInfo),  ZEND_STRL("url"));
    zval_dtor(&curlInfo);

    php_url *url_info = php_url_parse( Z_STRVAL_P(z_url) );
    char peer[200];
    char operation_name[500];

    int peer_port = 0;
    if(url_info->port){
        peer_port = url_info->port;
    }
    if (peer_port > 0) {
        sprintf(peer, "%s:%d", url_info->host, peer_port);
    } else {
        if (strcasecmp("http", url_info->scheme) == 0) {
            sprintf(peer, "%s:%d", url_info->host, 80);
        } else {
            sprintf(peer, "%s:%d", url_info->host, 443);
        }
    }

    if (url_info->query) {
        sprintf(operation_name, "%s?%s", url_info->path, url_info->query);
    } else {
        sprintf(operation_name, "%s", url_info->path);
    }

    zend_string *_peer_host,*_op_name;
    _peer_host = zend_string_init(peer, sizeof(peer) - 1, 0);
    _op_name = zend_string_init(operation_name, sizeof(operation_name) - 1, 0);

    char *sw3 = generate_sw3(Z_LVAL_P(span_id) + 1, _peer_host, _op_name);
    zend_string_release(_peer_host);
    zend_string_release(_op_name);

    if (sw3 != NULL) {
        zval z_sw3;
        array_init(&z_sw3);
        char headers_string[500];
        sprintf(headers_string, "sw3: %s", sw3);
        add_next_index_string(&z_sw3, headers_string);
//		//send setopt header
        zval f_name, p[3];
        ZVAL_COPY(&p[0], zid);
        ZVAL_LONG(&p[1], CURLOPT_HTTPHEADER);
        ZVAL_COPY(&p[2], &z_sw3);
        zval_dtor(&z_sw3);
        ZVAL_STRING(&f_name, "curl_setopt");
        zval return_function_value;
        call_user_function(CG(function_table), NULL, &f_name, &return_function_value, 3, p);
        zval_dtor(&return_function_value);
        zval_dtor(&f_name);
        zval_dtor(&p[0]);
        zval_dtor(&p[1]);
        zval_dtor(&p[2]);
    }
    efree(sw3);

	orig_curl_exec(INTERNAL_FUNCTION_PARAM_PASSTHRU);



	call_user_function(CG(function_table), NULL, &function_name, &curlInfo, 1, params);
	zval_dtor(&params[0]);
	zval_dtor(&function_name);

	zval *z_http_code;
	l_millisecond = get_millisecond();
	millisecond = zend_atol(l_millisecond, strlen(l_millisecond));
	efree(l_millisecond);

	z_http_code = zend_hash_str_find(Z_ARRVAL(curlInfo),  ZEND_STRL("http_code"));
    zval_dtor(&curlInfo);

	add_assoc_long(&temp, "endTime", millisecond);


	add_assoc_string(&temp, "operationName", url_info->path);
	add_assoc_string(&temp, "peer",  peer);

	php_url_free(url_info);


	if( Z_LVAL_P(z_http_code) !=200 ){
        add_assoc_long(&temp, "isError", 1);
	}else{
        add_assoc_long(&temp, "isError", 0);
	}
	zend_hash_next_index_insert(Z_ARRVAL_P(spans), &temp);

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

        zend_string *date_fmt, *_log_path, *_log_path_lower;
        time_t t;
        t = time(NULL);
        date_fmt = php_format_date("YmdHi", sizeof("YmdHi") - 1, t, 1);
        _log_path = zend_string_init(log_path, strlen(log_path), 0);
        _log_path_lower = php_string_tolower(_log_path);

        bzero(logFilename, 100);
        sprintf(logFilename, "%s/skywalking.%s.log", ZSTR_VAL(_log_path_lower), ZSTR_VAL(date_fmt));

        zend_string_release(date_fmt);
        zend_string_release(_log_path);
        zend_string_release(_log_path_lower);
        bzero(message, strlen(text));
        sprintf(message, "%s\n", text);
        _php_error_log_ex(3, message, strlen(message), logFilename, NULL);
    }

}


static char *generate_sw3(zend_long span_id, zend_string *peer_host, zend_string *operation_name) {

	char *sw3 = (char *) emalloc(sizeof(char) * 180 + ZSTR_LEN(peer_host) + ZSTR_LEN(operation_name));

	zval *traceId = zend_hash_str_find(Z_ARRVAL(SKYWALKING_G(context)), "currentTraceId", sizeof("currentTraceId") - 1);
	zval *entryApplicationInstance = zend_hash_str_find(Z_ARRVAL(SKYWALKING_G(context)), "entryApplicationInstance",
														sizeof("entryApplicationInstance") - 1);
	zval *entryOperationName = zend_hash_str_find(Z_ARRVAL(SKYWALKING_G(context)), "entryOperationName",
														sizeof("entryOperationName") - 1);
	zval *distributedTraceId = zend_hash_str_find(Z_ARRVAL(SKYWALKING_G(context)), "distributedTraceId",
														sizeof("distributedTraceId") - 1);

	sprintf(sw3, "%s|%d|%d|%d|#%s|#%s|#%s|%s", Z_STRVAL_P(traceId), span_id,
			application_instance, Z_LVAL_P(entryApplicationInstance), ZSTR_VAL(peer_host), Z_STRVAL_P(entryOperationName), ZSTR_VAL(operation_name), Z_STRVAL_P(distributedTraceId));
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

    if (sw3 != NULL && Z_TYPE_P(sw3) == IS_STRING) {
        add_assoc_string(&SKYWALKING_G(context), "sw3", Z_STRVAL_P(sw3));

        zval temp;
        array_init(&temp);

        php_explode(zend_string_init(ZEND_STRL("|"), 0), Z_STR_P(sw3), &temp, 10);

        zval *sw3_0 = zend_hash_index_find(Z_ARRVAL(temp), 0);
        zval *sw3_1 = zend_hash_index_find(Z_ARRVAL(temp), 1);
        zval *sw3_2 = zend_hash_index_find(Z_ARRVAL(temp), 2);
        zval *sw3_3 = zend_hash_index_find(Z_ARRVAL(temp), 3);
        zval *sw3_4 = zend_hash_index_find(Z_ARRVAL(temp), 4);
        zval *sw3_5 = zend_hash_index_find(Z_ARRVAL(temp), 5);
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
        add_assoc_string(&SKYWALKING_G(context), "distributedTraceId", Z_STRVAL_P(sw3_7));
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

    array_init(&SKYWALKING_G(context));
	array_init(&SKYWALKING_G(UpstreamSegment));

    generate_context();

    add_assoc_long(&SKYWALKING_G(UpstreamSegment), "application_instance", application_instance);
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
	array_init(&temp);

    add_assoc_long(&temp, "spanId", 0);
    add_assoc_long(&temp, "parentSpanId", -1);
    char *l_millisecond = get_millisecond();
    long millisecond = zend_atol(l_millisecond, strlen(l_millisecond));
    efree(l_millisecond);
    add_assoc_long(&temp, "startTime", millisecond);
    add_assoc_string(&temp, "operationName", get_page_request_uri());
    add_assoc_string(&temp, "peer", "");
    add_assoc_long(&temp, "spanType", 0);
    add_assoc_long(&temp, "spanLayer", 3);
    add_assoc_long(&temp, "componentId", COMPONENT_HTTPCLIENT);

    zval *isChild = zend_hash_str_find(Z_ARRVAL_P(&SKYWALKING_G(context)), "isChild", sizeof("isChild") - 1);
    // refs
    zval refs;
    array_init(&refs);
    if(Z_LVAL_P(isChild) == 1) {
        zval ref;
        array_init(&ref);
        zval *parentTraceSegmentId = zend_hash_str_find(Z_ARRVAL(SKYWALKING_G(context)), "parentTraceSegmentId", sizeof("parentTraceSegmentId") - 1);
        zval *parentSpanId = zend_hash_str_find(Z_ARRVAL(SKYWALKING_G(context)), "parentSpanId", sizeof("parentSpanId") - 1);
        zval *parentApplicationInstance = zend_hash_str_find(Z_ARRVAL(SKYWALKING_G(context)), "parentApplicationInstance", sizeof("parentApplicationInstance") - 1);
        zval *entryApplicationInstance = zend_hash_str_find(Z_ARRVAL(SKYWALKING_G(context)), "entryApplicationInstance", sizeof("entryApplicationInstance") - 1);
        zval *entryOperationName = zend_hash_str_find(Z_ARRVAL(SKYWALKING_G(context)), "entryOperationName", sizeof("entryOperationName") - 1);
        zval *networkAddress = zend_hash_str_find(Z_ARRVAL(SKYWALKING_G(context)), "networkAddress", sizeof("networkAddress") - 1);
        add_assoc_long(&ref, "type", 0);
        add_assoc_string(&ref, "parentTraceSegmentId", Z_STRVAL_P(parentTraceSegmentId));
        add_assoc_long(&ref, "parentSpanId", Z_LVAL_P(parentSpanId));
        add_assoc_long(&ref, "parentApplicationInstanceId", Z_LVAL_P(parentApplicationInstance));
        add_assoc_string(&ref, "networkAddress", Z_STRVAL_P(networkAddress));
        add_assoc_long(&ref, "entryApplicationInstanceId", Z_LVAL_P(entryApplicationInstance));
        add_assoc_string(&ref, "entryServiceName", Z_STRVAL_P(entryOperationName));
        add_assoc_string(&ref, "parentServiceName", get_page_request_uri());

        zend_hash_next_index_insert(Z_ARRVAL(refs), &ref);

    }

    zend_hash_str_add(Z_ARRVAL(temp), "refs", sizeof("refs") - 1, &refs);
    zend_hash_next_index_insert(Z_ARRVAL(spans), &temp);

    add_assoc_zval(&traceSegmentObject, "spans", &spans);

    zval globalTraceIds;
    array_init(&globalTraceIds);
    zval tmpGlobalTraceIds;
    ZVAL_STRING(&tmpGlobalTraceIds, Z_STRVAL_P(traceId));
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
    char uuid[37];
    uuid_t uuid1;
    uuid_generate_random(uuid1);

    uuid_unparse_lower(uuid1, uuid);

    char hostname[100] = {0};
    if (gethostname(hostname, sizeof(hostname)) < 0) {
        strcpy(hostname, "");
    }

    char *l_millisecond = get_millisecond();
    long millisecond = zend_atol(l_millisecond, strlen(l_millisecond));
    efree(l_millisecond);

    i = 0;
    do {
        application_instance = registerInstance(SKYWALKING_G(grpc), application_id, millisecond, uuid, SKY_OS_NAME,
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
//	ZEND_INIT_MODULE_GLOBALS(skywalking, php_skywalking_init_globals, NULL);
	//data_register_hashtable();
	REGISTER_INI_ENTRIES();
	/* If you have INI entries, uncomment these lines
	*/
	if (SKYWALKING_G(enable)) {
		module_init();
		if (sky_close == 1) {
			return SUCCESS;
		}
//		set_sampling_rate(SKY_G(global_sampling_rate));
		zend_function *old_function;
		if ((old_function = zend_hash_str_find_ptr(CG(function_table), "curl_exec", sizeof("curl_exec") - 1)) != NULL) {
			orig_curl_exec = old_function->internal_function.handler;
			old_function->internal_function.handler = sky_curl_exec_handler;
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
	php_info_print_table_header(2, "skywalking support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
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
