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
#include "grpc/common_struct.h"
#include "php_skywalking.h"
#include "ext/standard/url.h" /* for php_url */

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


extern int goSkyGrpc(char*host, int  method, ParamDataStruct* paramData, void (*callfuct)( AppInstance* ) );

/* If you declare any globals in php_skywalking.h uncomment this:
*/
ZEND_DECLARE_MODULE_GLOBALS(skywalking)

/* True global resources - no need for thread safety here */
static int le_skywalking;
static zval glob_skywalking_globals_list, glob_skywalking_globals_queue;
static char super_peer_host[50];
static int is_send_curl_header = 0;
static zval super_curl_header;
static AppInstance appInstance;
static int sky_close = 0;
static int sky_increment_id = 0;
static long percent_int;
pthread_t pid_consumer;

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_sky_create, 0, 0, 1)
	ZEND_ARG_INFO(0, appCode)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_sky_void_arginfo, 0, 0, 0)
	ZEND_ARG_INFO(0, appCode)
ZEND_END_ARG_INFO()
/* }}} */






/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini*/
PHP_INI_BEGIN()
	STD_PHP_INI_BOOLEAN("skywalking.auto_open",   	"1", PHP_INI_ALL, OnUpdateBool, global_auto_open, zend_skywalking_globals, skywalking_globals)
	STD_PHP_INI_ENTRY("skywalking.app_code", "", PHP_INI_ALL, OnUpdateString, global_app_code, zend_skywalking_globals, skywalking_globals)
    STD_PHP_INI_ENTRY("skywalking.log_path", "/tmp", PHP_INI_ALL, OnUpdateString, global_log_path, zend_skywalking_globals, skywalking_globals)
    STD_PHP_INI_ENTRY("skywalking.send_type", "0", PHP_INI_ALL, OnUpdateLong, global_send_type, zend_skywalking_globals, skywalking_globals)
    STD_PHP_INI_ENTRY("skywalking.sampling_rate", "/tmp", PHP_INI_ALL, OnUpdateString, global_sampling_rate, zend_skywalking_globals, skywalking_globals)
    STD_PHP_INI_ENTRY("skywalking.header_client_ip_name", "api", PHP_INI_ALL, OnUpdateString, global_header_client_ip_name, zend_skywalking_globals, skywalking_globals)
    STD_PHP_INI_ENTRY("skywalking.grpc_trace", "127.0.0.1:50051", PHP_INI_ALL, OnUpdateString, global_app_grpc_trace, zend_skywalking_globals, skywalking_globals)

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

void accel_sky_curl_init(INTERNAL_FUNCTION_PARAMETERS)
{
	char 	*url = NULL;
	size_t	 url_len = 0;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|s", &url, &url_len) == FAILURE) {
		return;
	}
	if( url != NULL ){
		php_url *url_info = php_url_parse(url);
		char *peer_host_s;
		peer_host_s = url_info->host;
		int peer_port = 0;
		if(url_info->port){
			peer_port = url_info->port;
		}
		if(peer_port > 0){
			sprintf(super_peer_host, "%s:%d", peer_host_s, peer_port);
		}else{
			sprintf(super_peer_host, "%s", peer_host_s);
		}
		php_url_free(url_info);
	}
	start_node_span_of_curl();

	orig_curl_init(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void accel_sky_curl_setopt(INTERNAL_FUNCTION_PARAMETERS)
{
	zval       *zid, *zvalue;
	zend_long        options;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rlz", &zid, &options, &zvalue) == FAILURE) {
		return;
	}

	if( options == CURLOPT_URL ){

		char 	*url = NULL;
		if( Z_TYPE_P(zvalue) != IS_STRING){
			return ;
		}
		url = Z_STRVAL_P(zvalue);
		php_url *url_info = php_url_parse(url);
		char *peer_host_s;
		peer_host_s = url_info->host;
		int peer_port = 0;
		if(url_info->port){
			peer_port = url_info->port;
		}
		if(peer_port > 0){
			sprintf(super_peer_host, "%s:%d", peer_host_s, peer_port);
		}else{
			sprintf(super_peer_host, "%s", peer_host_s);
		}
		php_url_free(url_info);
	}
	if( options == CURLOPT_HTTPHEADER ){
		char 	*url = NULL;
		if( Z_TYPE_P(zvalue) != IS_ARRAY){
			return ;
		}
		if(strlen(super_peer_host) != 0){
			make_span_curl_header(super_peer_host, zvalue);
			orig_curl_setopt(INTERNAL_FUNCTION_PARAM_PASSTHRU);
			is_send_curl_header = 1;
		}
		ZVAL_COPY(&super_curl_header, zvalue);
		RETURN_TRUE ;
	}
	orig_curl_setopt(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void accel_sky_curl_exec(INTERNAL_FUNCTION_PARAMETERS)
{
	zval		*zid;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zid) == FAILURE) {
		return;
	}
	if(!is_send_curl_header && super_peer_host != NULL){

		if(Z_TYPE(super_curl_header) == IS_UNDEF || Z_TYPE(super_curl_header) == IS_NULL){
			array_init(&super_curl_header);
		}
		//send setopt header
		zval function_name, params[3];
		ZVAL_COPY(&params[0], zid);
		ZVAL_LONG(&params[1], CURLOPT_HTTPHEADER);
		ZVAL_COPY(&params[2], &super_curl_header);
		ZVAL_STRING(&function_name,  "curl_setopt");
		zval  return_function_value;
		call_user_function(CG(function_table), NULL, &function_name, &return_function_value, 3, params);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[2]);
		zval_ptr_dtor(&super_curl_header);
	}
	orig_curl_exec(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	end_node_span_of_curl( zid );
}
/* {{{ php_skywalking_init_globals
 */
/* Uncomment this function if you have INI entries*/
static void php_skywalking_init_globals(zend_skywalking_globals *skywalking_globals)
{
	skywalking_globals->global_app_code = NULL;
	skywalking_globals->global_log_path = NULL;
	skywalking_globals->global_sampling_rate = 99.22;
	skywalking_globals->global_auto_open = 1;
	skywalking_globals->global_send_type = 1;
	skywalking_globals->global_header_client_ip_name = NULL;
	skywalking_globals->global_app_grpc_trace = NULL;
}



static char *sky_json_encode(zval *parameter){

	int return_code;
	smart_str buf = {0};
	zend_long options = 0;
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
	return ZSTR_VAL(buf.s);
}

static void make_span_curl_header(char *peer_host, zval *headers)
{
	char *headers_string, *tmp_headers_string;
	tmp_headers_string = build_SWheader_value(peer_host);
	headers_string = (char *)emalloc(sizeof(char)*250);
	sprintf(headers_string, "sw3: %s", tmp_headers_string);
   	add_next_index_string (headers, headers_string);
    efree(headers_string);
}



static void in_queue_log(){

	zval *all_node_data = zend_hash_str_find(Z_ARRVAL(glob_skywalking_globals_list),  ZEND_STRL("all_node_data"));
	sky_array_unshift(&glob_skywalking_globals_queue, all_node_data);
}


static void* empty_consumer_thread_entry(zval* param)
{

	zval *operand;
	zend_string *string_key;
	zend_ulong num_key;
	int g_SegTypeVal = 15;
	int num = 0;
	
    while( 1 )
    { 
    
    	if(zend_hash_num_elements(Z_ARRVAL(glob_skywalking_globals_queue)) > 0){
			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(glob_skywalking_globals_queue), num_key, string_key, operand) {
				send_grpc_param(operand);
				zend_hash_index_del(Z_ARRVAL(glob_skywalking_globals_queue), num_key);
			} ZEND_HASH_FOREACH_END();
		}
			
        usleep( 1000 * g_SegTypeVal );
    }

    return NULL;
}

static void* send_grpc_param(zval *all_node_data)
{
	zval *z_unique_id = zend_hash_str_find(Z_ARRVAL_P(all_node_data), ZEND_STRL(SKYWALKING_DISTRIBUTED_TRACEIDS));
	zval *operand;
	zend_string *string_key;
	zend_ulong num_key;

	TraceSegmentObjectStruct* traceSegment = (TraceSegmentObjectStruct *)malloc(sizeof(TraceSegmentObjectStruct));
	char *results = sky_json_encode( all_node_data );

	char str_id_parts[20];
	long id_parts;
	comList* com_list = create_com_list();
	
	if(zend_hash_num_elements(Z_ARRVAL_P(z_unique_id)) > 0){
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(z_unique_id), num_key, string_key, operand) {
			UniqueIdStruct *unique_id = (UniqueIdStruct *)malloc(sizeof(UniqueIdStruct));
			sprintf(str_id_parts, "%s", Z_STRVAL_P(operand));
			id_parts = zend_atol(str_id_parts, strlen(str_id_parts));
			unique_id->idParts = id_parts;
			add_com_list(com_list, unique_id);
		} ZEND_HASH_FOREACH_END();
	}
	traceSegment->traceSegmentIdList = com_list;
	traceSegment->segment = results;
	ParamDataStruct* paramData;
	paramData =  (ParamDataStruct *)emalloc(sizeof(ParamDataStruct));
	paramData->sendTraceSegmentParam = (SendTraceSegmentParamStruct *)emalloc(sizeof(SendTraceSegmentParamStruct));
	paramData->sendTraceSegmentParam->traceSegment = traceSegment;
	int global_send_type = SKY_G(global_send_type) ;
	if( global_send_type & S_SEND_TYPE_WRITE ){
		write_log( results);
	}
	if( global_send_type & S_SEND_TYPE_GRPC ){
		goSkyGrpc( SKY_G(global_app_grpc_trace), METHOD__SEND_TRACE_SEGMENT, paramData, NULL);
	}
	efree(paramData->sendTraceSegmentParam);
	efree(paramData);
	free(traceSegment);
}



static void *write_log(char *text){
	char *log_path;
	char logFilename[100];
	char message[strlen(text)];
	log_path = SKY_G(global_log_path);

	zend_string *date_fmt;
	time_t t;
	t = time(NULL);
	date_fmt = php_format_date("Ymd", 4, t, 1);

	bzero(logFilename, 100);
	sprintf(logFilename, "%s/skywalking.%s.log", ZSTR_VAL(php_string_tolower(zend_string_init(log_path, strlen(log_path), 0))), ZSTR_VAL(date_fmt));

	bzero(message, strlen(text));
	sprintf(message, "%s\n", text);
	_php_error_log_ex(3, message, strlen(message) + 1, logFilename, NULL);
}

/**
 * 获取接收到 SWTraceContext 的 header
// //  */
static zval receive_SWHeader_from_caller()
{
	zend_bool jit_initialization = PG(auto_globals_jit);
	zval *ret;
	zval sw_header_info;
	array_init(&sw_header_info);
	if (jit_initialization) {
		zend_string *server_str = zend_string_init("_SERVER", sizeof("_SERVER") - 1, 0);
		zend_is_auto_global(server_str);
		zend_string_release(server_str);
	}

	if ((ret = zend_hash_str_find(Z_ARRVAL_P(zend_hash_str_find(&EG(symbol_table), ZEND_STRL("_SERVER"))), ZEND_STRL("HTTP_SWTRACECONTEXT"))) != NULL) {
		zend_string *sw_header_text = 	zend_string_copy(Z_STR_P(ret));
		zval sw_header_info_z;
		zval *operand;
		zend_string *string_key;
		zend_ulong num_key;
		array_init(&sw_header_info_z);

		zend_string *delim = zend_string_init(ZEND_STRL("|"), 0);
		php_explode(delim, sw_header_text, &sw_header_info_z, 10);



		char *sw_header_info_keys[6] = {"TraceId", "SpanId", "AppCode", "PeerHost", "DistributedTraceIds", "IsSample"};
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(&sw_header_info_z), num_key, string_key, operand) {
			zend_hash_str_update(Z_ARRVAL(sw_header_info),   sw_header_info_keys[num_key], strlen(sw_header_info_keys[num_key]), operand);
		} ZEND_HASH_FOREACH_END();
	}

	add_assoc_zval(&glob_skywalking_globals_list, "_swHeaderInfo", &sw_header_info);
	return sw_header_info;

}

static void set_sampling_rate(double degrees){


	degrees = _php_math_round(degrees, 2, PHP_ROUND_HALF_UP);
	percent_int = (int)(degrees*100);
	if( percent_int <= 0){
		percent_int = 100;
	}
	if(percent_int > 10000){
		percent_int = 10000;
	}
	
}

static zval* generate_trace_id_for_array(int use_parent_tid, zval* z_trace_id){

	char *trace_id = generate_trace_id();
	if(use_parent_tid){
		trace_id = generate_parent_trace_id();
	}else{
		trace_id = generate_trace_id();
	}


	php_explode(zend_string_init(ZEND_STRL("."), 0), zend_string_init(trace_id, strlen(trace_id), 0), z_trace_id, 10);
	return z_trace_id;
}

static char *generate_parent_info_from_header(char *header_name){
	zval *ret;
	zval *sw_header_info;
	if( (sw_header_info = zend_hash_str_find(Z_ARRVAL(glob_skywalking_globals_list), "_swHeaderInfo", sizeof("_swHeaderInfo") - 1)) != NULL){
		if ((ret = zend_hash_str_find(Z_ARRVAL_P(sw_header_info), header_name, sizeof(header_name) - 1)) != NULL) {
			return Z_STRVAL_P(ret);
		}
	}
	return NULL;
}

static int have_parent_info_from_header(){
	zval *sw_header_info;
	if( (sw_header_info = zend_hash_str_find(Z_ARRVAL(glob_skywalking_globals_list), "_swHeaderInfo", sizeof("_swHeaderInfo") - 1)) != NULL){
		if(zend_hash_num_elements(Z_ARRVAL_P(sw_header_info)) > 0){
			return 1;
		}
	}
	return 0;
}

static char *generate_parent_trace_id(){
	zval *ret;
	zval *sw_header_info;
	if( (sw_header_info = zend_hash_str_find(Z_ARRVAL(glob_skywalking_globals_list), "_swHeaderInfo", sizeof("_swHeaderInfo") - 1)) != NULL){
		if ((ret = zend_hash_str_find(Z_ARRVAL_P(sw_header_info), "TraceId", sizeof("TraceId") - 1)) != NULL) {
			return Z_STRVAL_P(ret);
		}
	}
}

static char *generate_trace_id(){
	zval *sw_header_info, z__trace_id;
	zval *ret;
	char  *_trace_id;
	_trace_id = (char *)emalloc(sizeof(char)*180);
	bzero(_trace_id, 180);
	if( (sw_header_info = zend_hash_str_find(Z_ARRVAL(glob_skywalking_globals_list), "_swHeaderInfo", sizeof("_swHeaderInfo") - 1)) != NULL){
		if ((ret = zend_hash_str_find(Z_ARRVAL_P(sw_header_info), "TraceId", sizeof("TraceId") - 1)) != NULL) {
			zend_hash_str_update(Z_ARRVAL(glob_skywalking_globals_list), "_traceId", sizeof("_traceId") - 1, ret);
			return Z_STRVAL_P(ret);
		}
	}
	if ((ret = zend_hash_str_find(Z_ARRVAL(glob_skywalking_globals_list), "_traceId", sizeof("_traceId") - 1)) != NULL) {
		return Z_STRVAL_P(ret);
	}
	_trace_id = make_trace_id();
	ZVAL_STRING(&z__trace_id, _trace_id);
	zend_hash_str_update(Z_ARRVAL(glob_skywalking_globals_list), "_traceId", sizeof("_traceId") - 1, &z__trace_id);

	return _trace_id;

}


static char *generate_distributed_trace_ids(){
	zval *sw_header_info;
	zval *ret;
	if( (sw_header_info = zend_hash_str_find(Z_ARRVAL(glob_skywalking_globals_list), "_swHeaderInfo", sizeof("_swHeaderInfo") - 1)) != NULL){
		if ((ret = zend_hash_str_find(Z_ARRVAL_P(sw_header_info), "DistributedTraceIds", sizeof("DistributedTraceIds") - 1)) != NULL) {
			return Z_STRVAL_P(ret);
		}
	}
	if ((ret = zend_hash_str_find(Z_ARRVAL(glob_skywalking_globals_list), "DistributedTraceIds", sizeof("DistributedTraceIds") - 1)) != NULL) {
		if(Z_TYPE_P(ret)  != IS_NULL){
			return Z_STRVAL_P(ret);
		}
	}


	char *_distributed_trace_ids;
	_distributed_trace_ids  = (char *)emalloc(sizeof(char)*180);
	bzero(_distributed_trace_ids, 180);
	_distributed_trace_ids = make_trace_id();
	add_assoc_string(&glob_skywalking_globals_list, "DistributedTraceIds", _distributed_trace_ids);

	return _distributed_trace_ids;
}


static char *make_trace_id(){
	int app_instance_id = get_app_instance_id();
	int sys_pid = getpid();
	char *millisecond = get_millisecond();
	char *makeTraceId;
   	makeTraceId = (char *)emalloc(sizeof(char)*180);

   	bzero(makeTraceId, 80);

    sprintf(makeTraceId, "%d.%d.%s%d", app_instance_id, sys_pid, millisecond, sky_increment_id);

    return makeTraceId;
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



static char *get_page_url_and_peer(){

	zval *carrier = NULL;
	zval *s_https,*http_host, *request_uri;
	carrier = &PG(http_globals)[TRACK_VARS_SERVER];

	smart_str uri = {0};
	if (strcasecmp("cli", sapi_module.name) == 0){
   		smart_str_appendl(&uri, "cli", strlen("cli"));
   	}else{
		smart_str_appendl(&uri, "http", strlen("http"));
		s_https = zend_hash_str_find(Z_ARRVAL_P(carrier), "HTTPS", sizeof("HTTPS") - 1);

		if(s_https != NULL &&  Z_STRVAL_P(s_https) == "on"){
			smart_str_appendc(&uri, 's');
		}
		http_host = zend_hash_str_find(Z_ARRVAL_P(carrier), "HTTP_HOST", sizeof("HTTP_HOST") - 1);
		smart_str_appendl(&uri, "://", strlen("://"));
		smart_str_appendl(&uri, Z_STRVAL_P(http_host), strlen(Z_STRVAL_P(http_host)));
		request_uri = zend_hash_str_find(Z_ARRVAL_P(carrier), "REQUEST_URI", sizeof("REQUEST_URI") - 1);
		smart_str_appendl(&uri, Z_STRVAL_P(request_uri), strlen(Z_STRVAL_P(request_uri)));
	}
	smart_str_0(&uri);
	return ZSTR_VAL(uri.s);
}


static char *build_SWheader_value(const char *peer_host){

	char *trace_id, *app_code, *SW_header, *distributed_trace_ids;
	int span_id, parent_app_instance_id, entry_app_instance_id, entry_appname_operation_id, parent_appname_operation_id;
	char c_peer_host[55]  ;
	//zval SW_trace_context;
	SW_header = (char *)emalloc(sizeof(char)*250);
	trace_id = generate_trace_id();
	span_id = generate_span_id();
	parent_app_instance_id = get_app_instance_id();
	entry_app_instance_id = _entry_app_instance_id();
	app_code = SKY_G(global_app_code);
	sprintf(c_peer_host, "#%s", peer_host);
	entry_appname_operation_id = _entry_app_name_operation_id();
	parent_appname_operation_id = _parent_appname_operation_id();
	distributed_trace_ids = generate_distributed_trace_ids();
	

	sprintf(SW_header, "%s|%d|%d|%d|%s|%d|%d|%s", 
		trace_id, span_id, parent_app_instance_id, entry_app_instance_id, 
		c_peer_host, entry_appname_operation_id, parent_appname_operation_id, distributed_trace_ids
		);


	return SW_header;
}

static long generate_span_id(){

	long span_id;
	zval *swheaderinfo = zend_hash_str_find(Z_ARRVAL(glob_skywalking_globals_list), "_swHeaderInfo", sizeof("_swHeaderInfo") - 1);
	if (swheaderinfo == NULL || zend_hash_num_elements(Z_ARRVAL_P(swheaderinfo)) == 0){
		span_id = 1; 
	}else{
		span_id = Z_LVAL_P(zend_hash_str_find(Z_ARRVAL_P(swheaderinfo), "SpanId", sizeof("SpanId") - 1));
	}

	if(!span_id){
		span_id = 1; 
	}
	zval z_span_id;
	ZVAL_LONG(&z_span_id, ++span_id);
	zend_hash_str_update(Z_ARRVAL(glob_skywalking_globals_list), "_spanID", sizeof("_spanID") - 1, &z_span_id);
	
	return span_id;
}

static zend_always_inline zend_uchar is_sampling(){

	zval *z_p_is_sampling , sampling_rate;
	int is_sampling_int;

	zend_long rand_val;

	z_p_is_sampling = zend_hash_str_find(Z_ARRVAL(glob_skywalking_globals_list), "_isSampling", sizeof("_isSampling") - 1);

	if( z_p_is_sampling == NULL || ZVAL_IS_NULL(z_p_is_sampling) ){
		is_sampling_int = 0;
		
		ZVAL_LONG(&sampling_rate, percent_int);

		if(Z_LVAL(sampling_rate) < 10000){
#if PHP_VERSION_ID >= 71000
			rand_val = php_mt_rand_common(1, 10000);
#else
			if (!BG(mt_rand_is_seeded)) {
				php_mt_srand(GENERATE_SEED());
			}
			rand_val = (zend_long) (php_mt_rand() >> 1);
			RAND_RANGE(rand_val, 1, 10000, PHP_MT_RAND_MAX);
#endif
			if(Z_LVAL(sampling_rate) >= rand_val){
				is_sampling_int = 1;
			}
		}
		zval z_readonly;
		ZVAL_BOOL(&z_readonly, is_sampling_int);

		zend_hash_str_update(Z_ARRVAL(glob_skywalking_globals_list), "_isSampling", sizeof("_isSampling") - 1, &z_readonly);
		return Z_TYPE(z_readonly);
	}
	return Z_TYPE_P(z_p_is_sampling);
}

static char* _entry_app_name(){
	if(have_parent_info_from_header()){
		return generate_parent_info_from_header("EntryAppname");
	}else{
		return SKY_G(global_app_code);
	}
}

static int _entry_app_instance_id(){
	if(have_parent_info_from_header()){
		char *parent_app_instance_id = generate_parent_info_from_header("EntryAppInstanceid");

		return zend_atoi(parent_app_instance_id, (int)strlen(parent_app_instance_id));
	}else{
		return get_app_instance_id();
	}
}
static long _entry_app_name_operation_id(){
	if(have_parent_info_from_header()){
		char *app_name_operation_id = generate_parent_info_from_header("EntryAppnameOperationId");

		return zend_atol(app_name_operation_id, (int)strlen(app_name_operation_id));
	}else{
		zval *span_first_node_data  = zend_hash_str_find(Z_ARRVAL(glob_skywalking_globals_list),  ZEND_STRL("span_first_node_data"));
		return  Z_LVAL_P(zend_hash_str_find(Z_ARRVAL_P(span_first_node_data),  ZEND_STRL(SKYWALKING_SPAN_ID)));
	}
}

static long _parent_appname_operation_id(){
    zval *span_first_node_data  = zend_hash_str_find(Z_ARRVAL(glob_skywalking_globals_list),  ZEND_STRL("span_first_node_data"));
	return  Z_LVAL_P(zend_hash_str_find(Z_ARRVAL_P(span_first_node_data),  ZEND_STRL(SKYWALKING_SPAN_ID)));
}




void set_app_instance_id( AppInstance* appInste ){
	if( appInste != NULL ){
		appInstance.applicationInstanceId = appInste->applicationInstanceId;
	}
	
}
void set_app_id( AppInstance* appInste ){
	if( appInste != NULL ){
		appInstance.applicationId = appInste->applicationId;
	}
}

static int get_app_instance_id(){
	return appInstance.applicationInstanceId;
}
static int  get_app_id(){
	return appInstance.applicationId;
}

static int is_auto_open(){
	if(sky_close){
		return 0;
	}
	if((strcasecmp("cli", sapi_module.name) != 0) && (SG(sapi_headers).http_response_code != 200)){
		return 0;
	}
	if(SKY_G(global_auto_open)){
		return 1;
	}
	return 0;
}


/**
 * 当前机器ip
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

static zval* get_father_node_data(zval* father_nodes_data){

	
	
	if(!have_parent_info_from_header()){
		return NULL;
	}
	
	zval z_trace_id;
	array_init(&z_trace_id);
	add_assoc_zval(father_nodes_data, SKYWALKING_PARENT_TRACE_SEGMENT_ID, generate_trace_id_for_array(USE_PARENT_TRACE_ID, &z_trace_id));

	char *parent_app_instance_id = generate_parent_info_from_header("ParentAppInstanceid");
	add_assoc_long(father_nodes_data, SKYWALKING_PARENT_APPLICATION_ID, zend_atoi(parent_app_instance_id, strlen(parent_app_instance_id)));
	
	char *span_id = generate_parent_info_from_header("SpanId");
	add_assoc_long(father_nodes_data, SKYWALKING_PARENT_SPAN_ID, zend_atoi(span_id, strlen(span_id)));
	add_assoc_string(father_nodes_data, SKYWALKING_PARENT_SERVICE_ID, generate_parent_info_from_header("ParentAppname"));
	add_assoc_string(father_nodes_data, SKYWALKING_PARENT_SERVICE_NAME, generate_parent_info_from_header("ParentAppname"));
	add_assoc_string(father_nodes_data, SKYWALKING_NETWORK_ADDRESS_ID, generate_parent_info_from_header("PeerHost"));
	add_assoc_string(father_nodes_data, SKYWALKING_NETWORK_ADDRESS, generate_parent_info_from_header("PeerHost"));
	add_assoc_string(father_nodes_data, SKYWALKING_PARENT_SERVICE_ID, generate_parent_info_from_header("ParentAppname"));
	add_assoc_string(father_nodes_data, SKYWALKING_ENTRY_APPLICATION_INSTANCE_ID, generate_parent_info_from_header("EntryAppInstanceid"));
	char *entry_app_id = generate_parent_info_from_header("EntryAppId");
	add_assoc_long(father_nodes_data, SKYWALKING_ENTRY_SERVICE_ID, zend_atoi(entry_app_id, strlen(entry_app_id)));
	add_assoc_string(father_nodes_data, SKYWALKING_ENTRY_SERVICE_NAME, generate_parent_info_from_header("EntryAppname"));
	add_assoc_long(father_nodes_data, SKYWALKING_REF_TYPE_VALUE, 0);

	return father_nodes_data;
}

static zval *set_span_nodes_data(zval *node_data){
	zval *spans_node_data;
	spans_node_data = zend_hash_str_find(Z_ARRVAL(glob_skywalking_globals_list),  ZEND_STRL("_spansNodeData"));
	zend_hash_next_index_insert(Z_ARRVAL_P(spans_node_data), node_data);
	zval_ptr_dtor(node_data);
}

static void nodeinit(){

	if(!sky_live_pthread(pid_consumer)){
		if (pthread_create(&pid_consumer, NULL, (void *) empty_consumer_thread_entry, NULL) < 0)
		{
			sky_close  = 1;
			return ;
		}
	}
	zval glob_all_node_data, segment, span_first_node_data;
	zval father_node_data;
	array_init(&glob_all_node_data);
	array_init(&span_first_node_data);
	array_init(&segment);
	array_init(&father_node_data);

	receive_SWHeader_from_caller();
	zval z_trace_id;
	array_init(&z_trace_id);
	generate_trace_id_for_array(DONOT_USE_PARENT_TRACE_ID, &z_trace_id);
	add_assoc_zval(&glob_all_node_data, SKYWALKING_DISTRIBUTED_TRACEIDS, &z_trace_id);

	add_assoc_zval(&segment, SKYWALKING_TRACE_SEGMENT_ID, &z_trace_id);

	add_assoc_long(&segment, SKYWALKING_APPLICATION_ID, get_app_id());
	add_assoc_long(&segment, SKYWALKING_APPLICATION_INSTANCE_ID, get_app_instance_id());

	get_father_node_data(&father_node_data);
	
	add_assoc_zval(&segment, SKYWALKING_FATHER_NODE_DATA, &father_node_data);
	add_assoc_zval(&glob_all_node_data, SKYWALKING_SEGMENT, &segment);


	add_assoc_long(&span_first_node_data, SKYWALKING_SPAN_ID, generate_span_id());

	add_assoc_long(&span_first_node_data, SKYWALKING_SPAN_TYPE_VALUE, 0);
	add_assoc_long(&span_first_node_data, SKYWALKING_SPAN_LAYER_VALUE, 3);

	char *l_millisecond = get_millisecond();
    long starttime =   zend_atol(l_millisecond, strlen(l_millisecond));
    add_assoc_long(&span_first_node_data, SKYWALKING_STARTTIME, starttime);
    add_assoc_long(&span_first_node_data, SKYWALKING_FATHER_SPAN_ID, -1);

    //add_assoc_string(&span_first_node_data, SKYWALKING_COMPONENT_ID, "php-server");
    add_assoc_string(&span_first_node_data, SKYWALKING_COMPONENT_NAME, "php-server");
    //add_assoc_string(&span_first_node_data, SKYWALKING_PEER_ID, "server");
    add_assoc_string(&span_first_node_data, SKYWALKING_PEER, "127.0.0.1");
    SKY_ADD_ASSOC_ZVAL(&span_first_node_data, SKYWALKING_LOGS);
    
	//add_assoc_string(&span_first_node_data, SKYWALKING_OPERATION_NAME_ID, get_page_url_and_peer());
	add_assoc_string(&span_first_node_data, SKYWALKING_OPERATION_NAME, get_page_url_and_peer());
	add_assoc_bool(&span_first_node_data, SKYWALKING_IS_ERROR, 0);
	
	zval t_tags, t_tags_tmp;
	array_init(&t_tags);
	array_init(&t_tags_tmp);

	zval z_url_k, z_url_v;
	ZVAL_STRING(&z_url_k, "url");
	ZVAL_STRING(&z_url_v, get_page_url_and_peer());
	zend_hash_str_update(Z_ARRVAL(t_tags_tmp), "k", sizeof("k") - 1, &z_url_k);
	zend_hash_str_update(Z_ARRVAL(t_tags_tmp), "v", sizeof("v") - 1, &z_url_v);
	add_index_zval(&t_tags, 0, &t_tags_tmp);
	add_assoc_zval(&span_first_node_data, SKYWALKING_TAGS, &t_tags);

	add_assoc_zval(&glob_skywalking_globals_list, "span_first_node_data", &span_first_node_data);
	add_assoc_zval(&glob_skywalking_globals_list, "all_node_data", &glob_all_node_data);

	SKY_ADD_ASSOC_ZVAL(&glob_skywalking_globals_list, "_span_node_data");
	SKY_ADD_ASSOC_ZVAL(&glob_skywalking_globals_list, "_spansNodeData");
}


static void start_node_span_of_curl(){

	zval span_node_data, * z_span_id;
	array_init(&span_node_data);
	zval *span_first_node_data  = zend_hash_str_find(Z_ARRVAL(glob_skywalking_globals_list),  ZEND_STRL("span_first_node_data"));

	add_assoc_long(&span_node_data, SKYWALKING_SPAN_ID,  generate_span_id());
	add_assoc_long(&span_node_data, SKYWALKING_FATHER_SPAN_ID,  Z_LVAL_P(zend_hash_str_find(Z_ARRVAL_P(span_first_node_data),  ZEND_STRL(SKYWALKING_SPAN_ID))));

	char *l_millisecond = get_millisecond();
    long starttime =   zend_atol(l_millisecond, strlen(l_millisecond));
    add_assoc_long(&span_node_data, SKYWALKING_STARTTIME,  starttime);
    //add_assoc_string(&span_node_data, SKYWALKING_COMPONENT_ID,  "php-server");
    add_assoc_string(&span_node_data, SKYWALKING_COMPONENT_NAME,  "php-curl");
    add_assoc_long(&span_node_data, SKYWALKING_SPAN_TYPE_VALUE,  1);
    add_assoc_long(&span_node_data, SKYWALKING_SPAN_LAYER_VALUE, 3);

	zend_hash_str_update(Z_ARRVAL(glob_skywalking_globals_list), "_span_node_data", sizeof("_span_node_data") - 1, &span_node_data);
}

static void end_node_span_of_curl(zval *curl){


	zval *span_node_data;
	span_node_data = zend_hash_str_find(Z_ARRVAL(glob_skywalking_globals_list),  ZEND_STRL("_span_node_data"));

	zval function_name,curlInfo;
	zval params[1];
	ZVAL_COPY(&params[0], curl);
	ZVAL_STRING(&function_name,  "curl_getinfo");
	call_user_function(CG(function_table), NULL, &function_name, &curlInfo, 1, params);
	zval_ptr_dtor(&params[0]);


	zval *span_string_param, *span_first_node_data, *span_int_param, *span_bool_param,null_oject;
	zval *z_http_code;
	long millisecond;
	char *l_millisecond = get_millisecond();
	millisecond = zend_atol(l_millisecond, strlen(l_millisecond));

	zval *z_url = zend_hash_str_find(Z_ARRVAL(curlInfo),  ZEND_STRL("url"));

	z_http_code = zend_hash_str_find(Z_ARRVAL(curlInfo),  ZEND_STRL("http_code"));
	zval t_tags, t_tags_tmp;
	array_init(&t_tags);
	array_init(&t_tags_tmp);

	zval z_url_k;
	ZVAL_STRING(&z_url_k, "url");
	zend_hash_str_update(Z_ARRVAL(t_tags_tmp), "k", sizeof("k") - 1, &z_url_k);
	zend_hash_str_update(Z_ARRVAL(t_tags_tmp), "v", sizeof("v") - 1, z_url);
	add_index_zval(&t_tags, 0, &t_tags_tmp);
	
	add_assoc_long(span_node_data, SKYWALKING_ENDTIME, millisecond);

	php_url *url_info = php_url_parse( Z_STRVAL_P(z_url) );
	char peer_operation[200];

	int peer_port = 0;
	if(url_info->port){
		peer_port = url_info->port;
	}
	if(peer_port > 0){
		sprintf(peer_operation, "%s:%d%s", url_info->host, peer_port, url_info->path);
	}else{
		sprintf(peer_operation, "%s%s", url_info->host, url_info->path);
	}
	php_url_free(url_info);

    //add_assoc_string(&span_node_data, SKYWALKING_PEER_ID,  "server");
    add_assoc_string(span_node_data, SKYWALKING_PEER,  super_peer_host);
    add_assoc_string(span_node_data, SKYWALKING_OPERATION_NAME, peer_operation);
    SKY_ADD_ASSOC_ZVAL(span_node_data, SKYWALKING_LOGS);
	//add_assoc_string(span_node_data, SKYWALKING_OPERATION_NAME_ID, z_url);
	
	if( Z_LVAL_P(z_http_code) !=200 ){
		add_assoc_bool(span_node_data, SKYWALKING_IS_ERROR, 1);
	}else{
		add_assoc_bool(span_node_data, SKYWALKING_IS_ERROR, 0);
	}

	add_assoc_zval(span_node_data, SKYWALKING_TAGS, &t_tags);
	set_span_nodes_data(span_node_data);

}


static void *sky_flush_all(){
	zval *all_node_data, *span_first_node_data, *null_oject, span_bool_param, *span_int_param, *spans_node_data, *segment;
	all_node_data = zend_hash_str_find(Z_ARRVAL(glob_skywalking_globals_list),  ZEND_STRL("all_node_data"));
	segment = zend_hash_str_find(Z_ARRVAL_P(all_node_data),  ZEND_STRL(SKYWALKING_SEGMENT));

	zval *z_p_father_node_data = zend_hash_str_find(Z_ARRVAL_P(all_node_data),  ZEND_STRL(SKYWALKING_FATHER_NODE_DATA));
	//if (z_p_father_node_data == NULL || zend_hash_num_elements(Z_ARRVAL_P(z_p_father_node_data)) == 0){
	//	zend_hash_del(Z_ARRVAL_P(all_node_data), zend_string_init(ZEND_STRL(SKYWALKING_FATHER_NODE_DATA), 0));
	//}

	span_first_node_data = zend_hash_str_find(Z_ARRVAL(glob_skywalking_globals_list), ZEND_STRL("span_first_node_data"));
	char *l_millisecond = get_millisecond();
	add_assoc_long(span_first_node_data, SKYWALKING_ENDTIME, zend_atol(l_millisecond, strlen(l_millisecond)));

	spans_node_data = zend_hash_str_find(Z_ARRVAL(glob_skywalking_globals_list), ZEND_STRL("_spansNodeData"));

	sky_array_unshift(spans_node_data, span_first_node_data);
	char *l_f_millisecond = get_millisecond();
	//add_assoc_long(all_node_data, SKYWALKING_ENDTIME, zend_atol(l_f_millisecond, strlen(l_f_millisecond)));
	add_assoc_zval(segment, SKYWALKING_SPANS_NODE_DATA, spans_node_data);


	in_queue_log();
}


static void appinit(){

	array_init(&glob_skywalking_globals_queue);
	appInstance.applicationInstanceId = -100000;
	appInstance.applicationId = -100000;
	if (pthread_create(&pid_consumer, NULL, (void *) empty_consumer_thread_entry, NULL) < 0)
    {
        sky_close  = 1;
		return ;
    }

	char* app_code = SKY_G(global_app_code);
	char* ipv4s =  _get_current_machine_ip();
	char agentUUID[80];
	ParamDataStruct* paramData;
	paramData =  (ParamDataStruct *)emalloc(sizeof(ParamDataStruct));
	paramData->registerParam = (RegisterParamtruct *)emalloc(sizeof(RegisterParamtruct));
	paramData->registerParam->appCode = app_code;
	int stu = goSkyGrpc( SKY_G(global_app_grpc_trace), METHOD__GO_REGISTER, paramData, set_app_id);
	if( stu == 0 || appInstance.applicationId == -100000){
		sky_close  = 1;
		return ;
	}
	strcat(agentUUID, app_code);
	strcat(agentUUID, ipv4s);
	
	paramData->registerInstanceParam = (RegisterInstanceParamtruct *)emalloc(sizeof(RegisterInstanceParamtruct));
	paramData->registerInstanceParam->agentUUID = agentUUID;
	paramData->registerInstanceParam->osName =  SKY_OS_NAME;
	char hostname[100] = {0};
	if(gethostname(hostname, sizeof(hostname)) < 0){
		strcpy(hostname, "");  
    }
	paramData->registerInstanceParam->ipv4s = ipv4s;
	char *l_millisecond = get_millisecond();
    paramData->registerInstanceParam->registerTime =   zend_atol(l_millisecond, strlen(l_millisecond));
    paramData->registerInstanceParam->processNo = getpid();
    paramData->registerInstanceParam->hostname = hostname;
    paramData->registerInstanceParam->applicationId = appInstance.applicationId;
    

	stu = goSkyGrpc( SKY_G(global_app_grpc_trace), METHOD__GO_REGISTER_INSTANCE, paramData, set_app_instance_id);
	if( stu == 0 ||  appInstance.applicationInstanceId ==  -100000){
		//close  skywalking
		sky_close  = 1;
		return ;
	}
	efree( paramData->registerParam );
	efree( paramData );
}


/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(skywalking)
{
	ZEND_INIT_MODULE_GLOBALS(skywalking, php_skywalking_init_globals, NULL);
	if(sky_close){
		return SUCCESS;
	}
	//data_register_hashtable();
	REGISTER_INI_ENTRIES();

	/* If you have INI entries, uncomment these lines
	*/
	if(SKY_G(global_auto_open)){

		appinit();
		if( sky_close == 1 ){
			return SUCCESS;
		}
		set_sampling_rate(SKY_G(global_sampling_rate));
		zend_function *old_function;
		if ((old_function = zend_hash_str_find_ptr(CG(function_table), "curl_init", sizeof("curl_init")-1)) != NULL) {
			orig_curl_init = old_function->internal_function.handler;
			old_function->internal_function.handler = accel_sky_curl_init;
		}
		if ((old_function = zend_hash_str_find_ptr(CG(function_table), "curl_setopt", sizeof("curl_setopt")-1)) != NULL) {
			orig_curl_setopt = old_function->internal_function.handler;
			old_function->internal_function.handler = accel_sky_curl_setopt;
		}
		if ((old_function = zend_hash_str_find_ptr(CG(function_table), "curl_exec", sizeof("curl_exec")-1)) != NULL) {
			orig_curl_exec = old_function->internal_function.handler;
			old_function->internal_function.handler = accel_sky_curl_exec;
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
	/* uncomment this line if you have INI entries

	*/
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

	array_init(&glob_skywalking_globals_list);
	//&& is_sampling() == IS_TRUE
	if( is_auto_open()  ){
		sky_increment_id ++;
		if(sky_increment_id >= 999999){
			sky_increment_id = 0; 
		}
		nodeinit();
	}

	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(skywalking)
{
	if(is_auto_open()){
		sky_flush_all();
	}
	is_send_curl_header = 0;
	zval_ptr_dtor(&super_curl_header);
	array_init(&super_curl_header);

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


/** {{{ PHP_GINIT_FUNCTION

PHP_GINIT_FUNCTION(skywalking_globals)
{
	memset(skywalking_globals, 0, sizeof(*skywalking_globals));
}
 }}} */



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



/********************************************  transplantThe coming code *****************************************/

static comList* create_com_list(){
	comList  *com_list = (comList  *)malloc(sizeof(comList));
	memset(com_list, 0, sizeof(comList));
	com_list->count = 0 ;  
	com_list->head = NULL;
	com_list->tail = NULL;

	return com_list;
}

static int add_com_list(comList *com_list, UniqueIdStruct* data){

	comListNode *com_list_node = (comListNode *)malloc(sizeof(comListNode));;
	com_list_node->next = NULL;
	com_list_node->data = data;
	com_list->count ++;
	if(com_list->head == NULL){
		com_list->head = com_list_node;
	}else{
		com_list->tail->next = com_list_node;
	}
    com_list->tail = com_list_node;
	return 1;
}

static int sky_live_pthread(pthread_t tid) 
{

    int pthread_kill_err;

    pthread_kill_err = pthread_kill(tid, 0);
    if(pthread_kill_err == ESRCH){
		return 0;
    } else if(pthread_kill_err == EINVAL){
		return 0;
    } else {
		return 1;
    }
}

static long sky_array_unshift(zval *stack, zval *var)
{
	//7.0之前
	//php_splice(Z_ARRVAL_P(stack), 0, 0, args, argc, NULL TSRMLS_CC);
	HashTable new_hash;		/* New hashtable for the stack */

	zend_string *key;
	zval *value;


	zend_hash_init(&new_hash, zend_hash_num_elements(Z_ARRVAL_P(stack)) + 1, NULL, ZVAL_PTR_DTOR, 0);

	zend_hash_next_index_insert_new(&new_hash, var);
	if (EXPECTED(Z_ARRVAL_P(stack)->u.v.nIteratorsCount == 0)) {
		ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(stack), key, value) {
			if (key) {
				zend_hash_add_new(&new_hash, key, value);
			} else {
				zend_hash_next_index_insert_new(&new_hash, value);
			}
		} ZEND_HASH_FOREACH_END();
	} else {
		uint32_t old_idx;
		uint32_t new_idx = 0;
		uint32_t iter_pos = zend_hash_iterators_lower_pos(Z_ARRVAL_P(stack), 0);

		ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(stack), key, value) {
			if (key) {
				zend_hash_add_new(&new_hash, key, value);
			} else {
				zend_hash_next_index_insert_new(&new_hash, value);
			}
			old_idx = (Bucket*)value - Z_ARRVAL_P(stack)->arData;
			if (old_idx == iter_pos) {
				zend_hash_iterators_update(Z_ARRVAL_P(stack), old_idx, new_idx);
				iter_pos = zend_hash_iterators_lower_pos(Z_ARRVAL_P(stack), iter_pos + 1);
			}
			new_idx++;
		} ZEND_HASH_FOREACH_END();
	}


	Z_ARRVAL_P(stack)->u.v.nIteratorsCount = 0;
	Z_ARRVAL_P(stack)->pDestructor = NULL;
	zend_hash_destroy(Z_ARRVAL_P(stack));

	Z_ARRVAL_P(stack)->u.v.flags         = new_hash.u.v.flags;
	Z_ARRVAL_P(stack)->nTableSize        = new_hash.nTableSize;
	Z_ARRVAL_P(stack)->nTableMask        = new_hash.nTableMask;
	Z_ARRVAL_P(stack)->nNumUsed          = new_hash.nNumUsed;
	Z_ARRVAL_P(stack)->nNumOfElements    = new_hash.nNumOfElements;
	Z_ARRVAL_P(stack)->nNextFreeElement  = new_hash.nNextFreeElement;
	Z_ARRVAL_P(stack)->arData            = new_hash.arData;
	Z_ARRVAL_P(stack)->pDestructor       = new_hash.pDestructor;

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(stack));

	return zend_hash_num_elements(Z_ARRVAL_P(stack));
}
// /* }}} */





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
