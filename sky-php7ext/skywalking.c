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
#include "php_skywalking.h"
#include "ext/standard/url.h" /* for php_url */

#include "ext/standard/basic_functions.h"
#include "ext/standard/php_math.h"
#include <string.h>
#include "ext/json/php_json.h"
#include "ext/date/php_date.h"
#include <curl/curl.h>
#include <curl/easy.h>

static void (*orig_curl_init)(INTERNAL_FUNCTION_PARAMETERS) = NULL;
static void (*orig_curl_setopt)(INTERNAL_FUNCTION_PARAMETERS) = NULL;
static void (*orig_curl_exec)(INTERNAL_FUNCTION_PARAMETERS) = NULL;
/* If you declare any globals in php_skywalking.h uncomment this:

*/
ZEND_DECLARE_MODULE_GLOBALS(skywalking)

/* True global resources - no need for thread safety here */
static int le_skywalking;
char super_peer_host[50];
int is_send_curl_header = 0;
zval super_curl_header;

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
	STD_PHP_INI_ENTRY("skywalking.sampling_rate", "100", PHP_INI_ALL, OnUpdateReal, global_sampling_rate, zend_skywalking_globals, skywalking_globals)
    STD_PHP_INI_ENTRY("skywalking.log_path", "/tmp", PHP_INI_ALL, OnUpdateString, global_log_path, zend_skywalking_globals, skywalking_globals)
    STD_PHP_INI_ENTRY("skywalking.header_client_ip_name", "api", PHP_INI_ALL, OnUpdateString, global_header_client_ip_name, zend_skywalking_globals, skywalking_globals)
PHP_INI_END()

/* }}} */



/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */


zend_class_entry *skywalking_ce_entry;
zval sky_t_ptr;

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_skywalking_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_skywalking_compiled)
{

	char *arg = NULL;
	size_t arg_len, len;
	zend_string *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	strg = strpprintf(0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "skywalking", arg);

	RETURN_STR(strg);
}

PHP_METHOD(SkyWalking, __construct){


}
PHP_METHOD(SkyWalking, startSpan){
}
PHP_METHOD(SkyWalking, endSpan){
}
PHP_METHOD(SkyWalking, setLogPath){

	char *log_path;
	long log_path_len;
	zval *this_pr ;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &log_path, &log_path_len) == FAILURE) {
		return;
	}
	this_pr = getThis();

	zend_update_property_string(skywalking_ce_entry, this_pr, ZEND_STRL("_logPath"), log_path);

	RETURN_ZVAL(this_pr, 1, 0);
}
PHP_METHOD(SkyWalking, getInstance){
	char *appCode;
	long appCode_len;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|s", &appCode, &appCode_len) == FAILURE) {
		return;
	}

	zval *instance, rv = {{0}};
	if ((instance = sky_instance(&rv, appCode)) != NULL) {
		ZVAL_COPY(&sky_t_ptr, instance);
		RETURN_ZVAL(instance, 1, 0);
	}
}

PHP_METHOD(SkyWalking, setSamplingRate){
	double degrees;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "d", &degrees) == FAILURE) {
		return;
	}

	zval *this_ptr ;
	this_ptr = getThis();
	set_sampling_rate(this_ptr, degrees);


	RETURN_ZVAL(this_ptr, 1, 0);
}
PHP_METHOD(SkyWalking, startSpanOfCurl){

	char *peer_host;
	zval *headers;
	long peer_host_len,headers_len;
	zval *this_pr ;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "sz", &peer_host, &peer_host_len, &headers) == FAILURE) {
		return;
	}
	this_pr = getThis();
	start_span(this_pr);
	set_span_param_of_curl(peer_host, this_pr);
	Z_ADDREF_P(headers);
	make_span_curl_header(peer_host, headers, this_pr);

	RETURN_ZVAL(this_pr, 1, 0);
}




PHP_METHOD(SkyWalking, endSpanOfCurl){

	zval *this_ptr, *curl ;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &curl) == FAILURE) {
		return;
	}

	this_ptr = getThis();
	end_span_of_curl(this_ptr, curl);

	RETURN_ZVAL(this_ptr, 1, 0);
}

PHP_METHOD(SkyWalking, __finishAll){
	char *results;
	results =  sky_finishAll();
	RETVAL_STRING(results);
}


static void start_span(zval *this_pr){

	if(this_pr == NULL || is_sampling(this_pr) == IS_FALSE){
		return ;
	}

	zval span_node_data, *z_span_node_data_struct;
	zval z_span_id, millisecond;
	z_span_node_data_struct = zend_read_property(skywalking_ce_entry, this_pr, ZEND_STRL("_spanNodeDataStruct"), 1, NULL);
	array_init(&span_node_data);
	zend_hash_copy(Z_ARRVAL(span_node_data), Z_ARRVAL_P(z_span_node_data_struct), (copy_ctor_func_t) zval_add_ref);

	ZVAL_LONG(&z_span_id, generate_span_id(this_pr));
	char *l_millisecond = get_millisecond();
	ZVAL_LONG(&millisecond, zend_atol(l_millisecond, strlen(l_millisecond)));

	zend_hash_str_update(Z_ARRVAL(span_node_data), SKYWALKING_SPAN_ID, sizeof(SKYWALKING_SPAN_ID) - 1, &z_span_id);
	zend_hash_str_update(Z_ARRVAL(span_node_data), SKYWALKING_FATHER_SPAN_ID, sizeof(SKYWALKING_FATHER_SPAN_ID) - 1,
			zend_hash_str_find(Z_ARRVAL_P(zend_read_property(skywalking_ce_entry, this_pr, ZEND_STRL("_spanFirstNodeData"), 1, NULL)),
			ZEND_STRL(SKYWALKING_SPAN_ID)));
	zend_hash_str_update(Z_ARRVAL(span_node_data), SKYWALKING_STARTTIME, sizeof(SKYWALKING_STARTTIME) - 1, &millisecond);

	zend_update_property(skywalking_ce_entry, this_pr,  ZEND_STRL("_spanNodeData"), &span_node_data);
}

static void set_span_param_of_curl(char *peer_host, zval *this_pr)
{
	if(this_pr == NULL || is_sampling(this_pr) == IS_FALSE){
		return ;
	}

	zval *span_int_param,  *span_node_data, *span_string_param;
	span_node_data = zend_read_property(skywalking_ce_entry, this_pr, ZEND_STRL("_spanNodeData"), 1, NULL);
	span_int_param = zend_hash_find(Z_ARRVAL_P(span_node_data), zend_string_init(ZEND_STRL(SKYWALKING_SPAN_INT_PARAM), 0));
	span_string_param = zend_hash_find(Z_ARRVAL_P(span_node_data), zend_string_init(ZEND_STRL(SKYWALKING_SPAN_STRING_PARAM), 0));
	add_assoc_string(span_string_param, "span.layer", "http");
	add_assoc_string(span_string_param, "component", "php-curl");
	add_assoc_string(span_string_param, "span.kind", "client");

	int peer_port = 80 ;
	zval *peer_port_z;
	char *peer_host_s;
	if(_php_filter_validate_url(peer_host)){
		php_url *url_info = php_url_parse(peer_host);
		peer_host_s = url_info->host;
		if(url_info->port){
			peer_port = url_info->port;
		}
		php_url_free(url_info);
	}else{
		zval peer_host_array;
		array_init(&peer_host_array);
		zend_string *delim = zend_string_init(ZEND_STRL(":"), 0);
		php_explode(delim, zend_string_init( peer_host, strlen(peer_host), 0), &peer_host_array, 5);
		peer_host_s = Z_STRVAL_P(zend_hash_index_find(Z_ARRVAL(peer_host_array), 0));
		if((peer_port_z = zend_hash_index_find(Z_ARRVAL(peer_host_array), 1)) != NULL && Z_TYPE_P(peer_port_z) == IS_LONG){
			peer_port = Z_LVAL_P(peer_port_z);
		}
	}
	add_assoc_string(span_string_param, "peer.host", peer_host_s);
	add_assoc_long(span_int_param, "peer.port", (long)peer_port);
}

static void make_span_curl_header(char *peer_host, zval *headers, zval *this_pr)
{
	char *headers_string, *tmp_headers_string;
	tmp_headers_string = build_SWheader_value(peer_host, this_pr);
	headers_string = (char *)emalloc(sizeof(char)*250);
	sprintf(headers_string, "SWTraceContext: %s", tmp_headers_string);
   	add_next_index_string (headers, headers_string);
    efree(headers_string);
}


static void end_span_of_curl(zval *this_ptr,zval *curl){

	if(this_ptr == NULL){
		return ;
	}


	if(is_sampling(this_ptr) == IS_FALSE){
		return ;
	}

	zval *span_node_data;
	span_node_data = zend_read_property(skywalking_ce_entry, this_ptr, ZEND_STRL("_spanNodeData"), 1, NULL);

	if(Z_TYPE_P(curl) != IS_RESOURCE ||
		strcmp( ZSTR_VAL(php_string_tolower(zend_string_init(ZEND_STRL(zend_rsrc_list_get_rsrc_type(Z_RES_P(curl))), 0))), "curl") != 0 ){
		zend_throw_exception(NULL, "Error ：Need setting curl Object", 0);
	}
	zval function_name,curlInfo;
	zval params[1];
	ZVAL_COPY(&params[0], curl);
	ZVAL_STRING(&function_name,  "curl_getinfo");
	call_user_function(CG(function_table), NULL, &function_name, &curlInfo, 1, params);
	zval_ptr_dtor(&params[0]);


	zval *span_string_param, *span_first_node_data, *span_int_param, *span_bool_param,null_oject;
	zval *z_http_code;
	zval millisecond;
	char *l_millisecond = get_millisecond();
	ZVAL_LONG(&millisecond, zend_atol(l_millisecond, strlen(l_millisecond)));

	zend_hash_str_update(Z_ARRVAL_P(span_node_data), SKYWALKING_ENDTIME, sizeof(SKYWALKING_ENDTIME) - 1, &millisecond);
	zend_hash_str_update(Z_ARRVAL_P(span_node_data), SKYWALKING_SPAN_SERVER_URI, sizeof(SKYWALKING_SPAN_SERVER_URI) - 1,
			zend_hash_str_find(Z_ARRVAL(curlInfo),  ZEND_STRL("url")));

	z_http_code = zend_hash_str_find(Z_ARRVAL(curlInfo),  ZEND_STRL("http_code"));
	span_int_param = zend_hash_find(Z_ARRVAL_P(span_node_data),  zend_string_init(ZEND_STRL(SKYWALKING_SPAN_INT_PARAM), 0));
	add_assoc_long(span_int_param, "status_code", Z_LVAL_P(z_http_code));

	span_bool_param = zend_hash_find(Z_ARRVAL_P(span_node_data), zend_string_init(ZEND_STRL(SKYWALKING_SPAN_BOOL_PARAM), 0));
	if( Z_LVAL_P(z_http_code) !=200 ){
		add_assoc_string(span_bool_param, "error", "true");
	}

	if (span_bool_param == NULL ||  zend_hash_num_elements(Z_ARRVAL_P(span_bool_param)) == 0){
		object_init(&null_oject);
		ZVAL_COPY(span_bool_param, &null_oject);
	}
	span_string_param = zend_hash_find(Z_ARRVAL_P(span_node_data), zend_string_init(ZEND_STRL(SKYWALKING_SPAN_STRING_PARAM), 0));
	span_int_param = zend_hash_find(Z_ARRVAL_P(span_node_data), zend_string_init(ZEND_STRL(SKYWALKING_SPAN_INT_PARAM), 0));
	add_assoc_string(span_string_param, "url", Z_STRVAL_P(zend_hash_str_find(Z_ARRVAL(curlInfo),  ZEND_STRL("url"))));


	set_span_nodes_data(this_ptr, span_node_data);
}

static char *sky_finishAll(){

	if(is_sampling(&sky_t_ptr) == IS_FALSE){
		return "";
	}

	zval *all_node_data, *span_first_node_data, *null_oject, span_bool_param, *span_int_param, *spans_node_data;
	all_node_data = zend_read_property(skywalking_ce_entry,  &sky_t_ptr, ZEND_STRL("_allNodeData"), 1, NULL);
	zval *z_p_father_node_data = zend_hash_find(Z_ARRVAL_P(all_node_data), zend_string_init(ZEND_STRL(SKYWALKING_FATHER_NODE_DATA), 0));
	if (z_p_father_node_data == NULL || zend_hash_num_elements(Z_ARRVAL_P(z_p_father_node_data)) == 0){
		zend_hash_del(Z_ARRVAL_P(all_node_data), zend_string_init(ZEND_STRL(SKYWALKING_FATHER_NODE_DATA), 0));
	}
	span_first_node_data = zend_read_property(skywalking_ce_entry,  &sky_t_ptr, ZEND_STRL("_spanFirstNodeData"), 1, NULL);
	char *l_millisecond = get_millisecond();
	add_assoc_long(span_first_node_data, SKYWALKING_ENDTIME, zend_atol(l_millisecond, strlen(l_millisecond)));

	object_init(&span_bool_param);
	span_int_param = zend_hash_find(Z_ARRVAL_P( span_first_node_data), zend_string_init(ZEND_STRL(SKYWALKING_SPAN_INT_PARAM), 0));
	add_assoc_zval(span_first_node_data, SKYWALKING_SPAN_BOOL_PARAM, &span_bool_param);
	add_assoc_long(span_int_param, "status_code", 200);
	add_assoc_long(span_int_param, "peer.port", 80);
	spans_node_data = zend_read_property(skywalking_ce_entry, &sky_t_ptr, ZEND_STRL("_spansNodeData"), 1, NULL);

	sky_array_unshift(spans_node_data, span_first_node_data);
	char *l_f_millisecond = get_millisecond();
	add_assoc_long(all_node_data, SKYWALKING_ENDTIME, zend_atol(l_f_millisecond, strlen(l_f_millisecond)));
	add_assoc_zval(all_node_data, SKYWALKING_SPANS_NODE_DATA, spans_node_data);
	char *results;
	results = sky_json_encode( all_node_data );

	write_log(&sky_t_ptr, results);

	return results;
}





/* {{{ skywalking_functions[]
 *
 * Every user visible function must have an entry in skywalking_functions[].
 */
const zend_function_entry skywalking_functions[] = {
	PHP_FE(confirm_skywalking_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE_END	/* Must be the last line in skywalking_functions[] */
};
/* }}} */



const zend_function_entry class_skywalking[] = {
	PHP_ME(SkyWalking,			__construct,		arginfo_sky_create, ZEND_ACC_CTOR|ZEND_ACC_PRIVATE)
	PHP_ME(SkyWalking,			getInstance,		arginfo_sky_void_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(SkyWalking,			startSpan,		NULL, ZEND_ACC_PUBLIC )
	PHP_ME(SkyWalking,			endSpan,		NULL, ZEND_ACC_PUBLIC )
	PHP_ME(SkyWalking,			setLogPath,		NULL, ZEND_ACC_PUBLIC )
	PHP_ME(SkyWalking,			setSamplingRate,		NULL, ZEND_ACC_PUBLIC )
	PHP_ME(SkyWalking,			startSpanOfCurl,		NULL, ZEND_ACC_PUBLIC )
	PHP_ME(SkyWalking,			endSpanOfCurl,		NULL, ZEND_ACC_PUBLIC )
	PHP_ME(SkyWalking,			__finishAll,		NULL, ZEND_ACC_PUBLIC )
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
	start_span(&sky_t_ptr);
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
		set_span_param_of_curl(super_peer_host, &sky_t_ptr);
	}

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
		set_span_param_of_curl(super_peer_host, &sky_t_ptr);
	}
	if( options == CURLOPT_HTTPHEADER ){
		char 	*url = NULL;
		if( Z_TYPE_P(zvalue) != IS_ARRAY){
			return ;
		}
		if(super_peer_host != NULL){
			make_span_curl_header(super_peer_host, zvalue, &sky_t_ptr);
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
	end_span_of_curl(&sky_t_ptr, zid);
}
/* {{{ php_skywalking_init_globals
 */
/* Uncomment this function if you have INI entries*/
static void php_skywalking_init_globals(zend_skywalking_globals *skywalking_globals)
{
	skywalking_globals->global_app_code = NULL;
	skywalking_globals->global_log_path = NULL;
	skywalking_globals->global_auto_open = 1;
	skywalking_globals->global_header_client_ip_name = NULL;
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
/* }}} */


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


static void *write_log(zval *this_ptr, char *text){
	char *log_path;
	char logFilename[100];
	char message[strlen(text)];
	log_path = Z_STRVAL_P(zend_read_property(skywalking_ce_entry, this_ptr, ZEND_STRL("_logPath"), 1, NULL));

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


//移植过来的代码
static int _php_filter_validate_url(char *value) /* {{{ */
{

	php_url *url;
	zend_long flags = 0x2000000;


	url = php_url_parse(value);

	if (url == NULL) {
		return 0;
	}

	if (url->scheme != NULL && (!strcasecmp(url->scheme, "http") || !strcasecmp(url->scheme, "https"))) {
		char *e, *s, *t;
		size_t l;

		if (url->host == NULL) {
			goto bad_url;
		}

		s = url->host;
		l = strlen(s);
		e = url->host + l;
		t = e - 1;

		if (*s == '[' && *t == ']' && _php_filter_validate_ipv6((s + 1), l - 2)) {
			php_url_free(url);
			return 1;
		}

		if (!_php_filter_validate_domain(url->host, l, 0x100000)) {
			php_url_free(url);
			return 0;
		}
	}

	if (
		url->scheme == NULL ||
		(url->host == NULL && (strcmp(url->scheme, "mailto") && strcmp(url->scheme, "news") && strcmp(url->scheme, "file"))) ||
		((flags & 0x040000) && url->path == NULL) || ((flags & 0x080000) && url->query == NULL)
	) {
bad_url:
		php_url_free(url);
		return 0;
	}
	php_url_free(url);
	return 1;
}
/* }}} */



static int _php_filter_validate_domain(char * domain, int len, zend_long flags) /* {{{ */
{
	char *e, *s, *t;
	size_t l;
	int hostname = flags & 0x100000;
	unsigned char i = 1;

	s = domain;
	l = len;
	e = domain + l;
	t = e - 1;

	if (*t == '.') {
		e = t;
		l--;
	}

	if (l > 253) {
		return 0;
	}

	if(*s == '.' || (hostname && !isalnum((int)*(unsigned char *)s))) {
		return 0;
	}

	while (s < e) {
		if (*s == '.') {
			if (*(s + 1) == '.' || (hostname && (!isalnum((int)*(unsigned char *)(s - 1)) || !isalnum((int)*(unsigned char *)(s + 1))))) {
				return 0;
			}

			i = 1;
		} else {
			if (i > 63 || (hostname && *s != '-' && !isalnum((int)*(unsigned char *)s))) {
				return 0;
			}

			i++;
		}

		s++;
	}

	return 1;
}
/* }}} */



static int _php_filter_validate_ipv4(char *str, size_t str_len, int *ip) /* {{{ */
{

	const char *end = str + str_len;
	int num, m;
	int n = 0;

	while (str < end) {
		int leading_zero;
		if (*str < '0' || *str > '9') {
			return 0;
		}
		leading_zero = (*str == '0');
		m = 1;
		num = ((*(str++)) - '0');
		while (str < end && (*str >= '0' && *str <= '9')) {
			num = num * 10 + ((*(str++)) - '0');
			if (num > 255 || ++m > 3) {
				return 0;
			}
		}

		if (leading_zero && (num != 0 || m > 1))
			return 0;
		ip[n++] = num;
		if (n == 4) {
			return str == end;
		} else if (str >= end || *(str++) != '.') {
			return 0;
		}
	}
	return 0;
}
/* }}} */



static int _php_filter_validate_ipv6(char *str, size_t str_len) /* {{{ */
{
	int compressed = 0;
	int blocks = 0;
	int n;
	char *ipv4;
	char *end;
	int ip4elm[4];
	char *s = str;

	if (!memchr(str, ':', str_len)) {
		return 0;
	}

	ipv4 = memchr(str, '.', str_len);
	if (ipv4) {
 		while (ipv4 > str && *(ipv4-1) != ':') {
			ipv4--;
		}

		if (!_php_filter_validate_ipv4(ipv4, (str_len - (ipv4 - str)), ip4elm)) {
			return 0;
		}

		str_len = ipv4 - str;
		if (str_len < 2) {
			return 0;
		}

		if (ipv4[-2] != ':') {
			str_len--;
		}

		blocks = 2;
	}

	end = str + str_len;

	while (str < end) {
		if (*str == ':') {
			if (++str >= end) {

				return 0;
			}
			if (*str == ':') {
				if (compressed) {
					return 0;
				}
				blocks++;
				compressed = 1;

				if (++str == end) {
					return (blocks <= 8);
				}
			} else if ((str - 1) == s) {
				return 0;
			}
		}
		n = 0;
		while ((str < end) &&
		       ((*str >= '0' && *str <= '9') ||
		        (*str >= 'a' && *str <= 'f') ||
		        (*str >= 'A' && *str <= 'F'))) {
			n++;
			str++;
		}
		if (n < 1 || n > 4) {
			return 0;
		}
		if (++blocks > 8)
			return 0;
	}
	return ((compressed && blocks <= 8) || blocks == 8);
}
/* }}} */

static void date_register_classes(TSRMLS_D)
{

	zend_class_entry ce_skywalking, ce_immutable, ce_timezone, ce_interval, ce_period, ce_interface;

	INIT_CLASS_ENTRY(ce_skywalking, "SkyWalking", class_skywalking);
	skywalking_ce_entry = zend_register_internal_class_ex(&ce_skywalking, NULL);

	zend_declare_property_null(skywalking_ce_entry, ZEND_STRL("_instance"),	 ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);

	zend_declare_property_string(skywalking_ce_entry, "_appCode", sizeof("_appCode")-1, "", ZEND_ACC_PRIVATE);

	zend_declare_property_null(skywalking_ce_entry, "_allPartsNodesStruct", sizeof("_allPartsNodesStruct")-1,   ZEND_ACC_PRIVATE);
	zend_declare_property_null(skywalking_ce_entry, "_fatherNodesStruct", sizeof("_fatherNodesStruct")-1,   ZEND_ACC_PRIVATE);
	zend_declare_property_null(skywalking_ce_entry, "_spanNodeDataStruct", sizeof("_spanNodeDataStruct")-1,   ZEND_ACC_PRIVATE);


	zend_declare_property_null(skywalking_ce_entry, "_allNodeData", sizeof("_allNodeData")-1,   ZEND_ACC_PRIVATE);
	zend_declare_property_null(skywalking_ce_entry, "_spansNodeData", sizeof("_spansNodeData")-1,   ZEND_ACC_PRIVATE);
	zend_declare_property_null(skywalking_ce_entry, "_fatherNodeData", sizeof("_fatherNodeData")-1,   ZEND_ACC_PRIVATE);
	zend_declare_property_null(skywalking_ce_entry, "_spanNodeData", sizeof("_spanNodeData")-1,   ZEND_ACC_PRIVATE);
	zend_declare_property_null(skywalking_ce_entry, "_spanFirstNodeData", sizeof("_spanFirstNodeData")-1,   ZEND_ACC_PRIVATE);
	zend_declare_property_null(skywalking_ce_entry, "_swHeaderInfo", sizeof("_swHeaderInfo")-1,   ZEND_ACC_PRIVATE);

	zend_declare_property_null(skywalking_ce_entry, "_swHeaderText", sizeof("_swHeaderText")-1,  ZEND_ACC_PRIVATE);
	//唯一的事务编号
	zend_declare_property_null(skywalking_ce_entry, "_traceId", sizeof("_traceId")-1, ZEND_ACC_PRIVATE);
	zend_declare_property_null(skywalking_ce_entry, "_distributedTraceIds", sizeof("_distributedTraceIds")-1, ZEND_ACC_PRIVATE);
	//生产的SPAN_ID
	zend_declare_property_long(skywalking_ce_entry, "_spanID", sizeof("_spanID")-1, 0,  ZEND_ACC_PRIVATE);
	//日志路径
	zend_declare_property_null(skywalking_ce_entry, "_logPath", sizeof("_logPath")-1, ZEND_ACC_PRIVATE);
	//采样率
	zend_declare_property_null(skywalking_ce_entry, "_samplingRate", sizeof("_samplingRate")-1, ZEND_ACC_PRIVATE);
	//本次进程是否采样
	zend_declare_property_null(skywalking_ce_entry, "_isSampling", sizeof("_isSampling")-1, ZEND_ACC_PRIVATE);
}
/* }}} */


//实现单例
static zval *sky_instance(zval *this_ptr, const char *appCode) /* {{{ */ {

	zval *instance;
	//获取对象值  如果存在返回对象实例
	instance = zend_read_static_property(skywalking_ce_entry, ZEND_STRL("_instance"), 1);
	if (IS_OBJECT == Z_TYPE_P(instance)
			&& instanceof_function(Z_OBJCE_P(instance), skywalking_ce_entry)) {
		return instance;
	}
	//判断是否定义
	if (Z_ISUNDEF_P(this_ptr)) {
		object_init_ex(this_ptr, skywalking_ce_entry);
	} else {
		return this_ptr;
	}
	_init(appCode, this_ptr);


	return this_ptr;
}
/* }}} */

static void _init(const char *appCode, zval *this_ptr){


	//抛出异常
	if(appCode == NULL){
		zend_throw_exception(NULL, "Error ： Must set appCode", 0);
	}
	//更新_appCode属性值
	zend_update_static_property(skywalking_ce_entry, ZEND_STRL("_instance"), this_ptr);

	zval allparts_property, father_property, span_property;
	array_init(&allparts_property);
	array_init(&father_property);
	array_init(&span_property);

	/**
	 *   全部节点结构
     */
  	add_assoc_null(&allparts_property, SKYWALKING_TRACEID);

  	add_assoc_null(&allparts_property, SKYWALKING_STARTTIME);
  	add_assoc_null(&allparts_property, SKYWALKING_ENDTIME);
  	SKY_ADD_ASSOC_ZVAL(&allparts_property, SKYWALKING_FATHER_NODE_DATA);
  	SKY_ADD_ASSOC_ZVAL(&allparts_property, SKYWALKING_SPANS_NODE_DATA);
  	//add_assoc_zval(&allparts_property, SKYWALKING_FATHER_NODE_DATA, &null_array);
  	//add_assoc_zval(&allparts_property, SKYWALKING_SPANS_NODE_DATA, &null_array);
  	add_assoc_null(&allparts_property, SKYWALKING_APP_CODE);
  	add_assoc_null(&allparts_property, SKYWALKING_DISTRIBUTED_TRACEIDS);
  	zend_update_property(skywalking_ce_entry, this_ptr,  ZEND_STRL("_allPartsNodesStruct"), &allparts_property);
  	zval_ptr_dtor(&allparts_property);

	/**
     * 父节点数据结构
     */
  	add_assoc_null(&father_property, SKYWALKING_TRACEID);
  	add_assoc_null(&father_property, SKYWALKING_SPAN_ID);
  	add_assoc_null(&father_property, SKYWALKING_APP_CODE);
  	add_assoc_null(&father_property, SKYWALKING_PEERHOST);
	zend_update_property(skywalking_ce_entry, this_ptr, ZEND_STRL("_fatherNodesStruct"), &father_property);
	zval_ptr_dtor(&father_property);


	/**
     * span节点结构
     */
	add_assoc_null(&span_property, SKYWALKING_SPAN_ID);
  	add_assoc_long(&span_property, SKYWALKING_FATHER_SPAN_ID, -1);
  	add_assoc_null(&span_property, SKYWALKING_STARTTIME);
  	add_assoc_null(&span_property, SKYWALKING_ENDTIME);
  	add_assoc_string(&span_property, SKYWALKING_SPAN_SERVER_URI, "");
  	SKY_ADD_ASSOC_ZVAL(&span_property, SKYWALKING_SPAN_STRING_PARAM);
  	SKY_ADD_ASSOC_ZVAL(&span_property, SKYWALKING_SPAN_BOOL_PARAM);
  	SKY_ADD_ASSOC_ZVAL(&span_property, SKYWALKING_SPAN_INT_PARAM);
  	SKY_ADD_ASSOC_ZVAL(&span_property, SKYWALKING_SPAN_LOG);


	zend_update_property(skywalking_ce_entry,  this_ptr, ZEND_STRL("_spanNodeDataStruct"), &span_property);
	zval_ptr_dtor(&span_property);

	SKY_UPDATE_PROPERTY(skywalking_ce_entry, this_ptr, ZEND_STRL("_allNodeData"));
	//span节点的集合 数组
	SKY_UPDATE_PROPERTY(skywalking_ce_entry, this_ptr, ZEND_STRL("_spansNodeData"));
	//父节点数据
	SKY_UPDATE_PROPERTY(skywalking_ce_entry, this_ptr, ZEND_STRL("_fatherNodeData"));
	//单个span节点数据
	SKY_UPDATE_PROPERTY(skywalking_ce_entry, this_ptr, ZEND_STRL("_spanNodeData"));
	//第一个span节点数据
	SKY_UPDATE_PROPERTY(skywalking_ce_entry, this_ptr, ZEND_STRL("_spanFirstNodeData"));

	//kyWalking 包含的头信息
	SKY_UPDATE_PROPERTY(skywalking_ce_entry, this_ptr, ZEND_STRL("_swHeaderInfo"));

	zend_update_property_string(skywalking_ce_entry, this_ptr, ZEND_STRL("_appCode"), appCode);

	//对节点数据进行结构初始化  
	zval *all_parts_nodes_struct, all_node_data;
	all_parts_nodes_struct = zend_read_property(skywalking_ce_entry, this_ptr, ZEND_STRL("_allPartsNodesStruct"), 1, NULL);
	array_init(&all_node_data);
	zend_hash_copy(Z_ARRVAL(all_node_data), Z_ARRVAL_P(all_parts_nodes_struct), (copy_ctor_func_t) zval_add_ref);

	receive_SWHeader_from_caller(this_ptr);
	// //设置最开始的时间
	zval distributed_trace_ids_array;
	char *distributed_trace_ids;
	zval trace_id, all_millisecond;

	ZVAL_STRING(&trace_id, generate_trace_id(this_ptr));
	char *l_millisecond = get_millisecond();
	ZVAL_LONG(&all_millisecond, zend_atol(l_millisecond, strlen(l_millisecond)));

	zend_hash_str_update(Z_ARRVAL(all_node_data), SKYWALKING_TRACEID, sizeof(SKYWALKING_TRACEID) - 1, &trace_id);
	zend_hash_str_update(Z_ARRVAL(all_node_data), SKYWALKING_STARTTIME, sizeof(SKYWALKING_STARTTIME) - 1, &all_millisecond);
	zend_hash_str_update(Z_ARRVAL(all_node_data), SKYWALKING_APP_CODE, sizeof(SKYWALKING_APP_CODE) - 1,
			zend_read_property(skywalking_ce_entry, this_ptr, ZEND_STRL("_appCode"), 1, NULL));
	distributed_trace_ids = generate_distributed_trace_ids(this_ptr);

	array_init(&distributed_trace_ids_array);
	add_index_string(&distributed_trace_ids_array, 0, distributed_trace_ids);
	zend_hash_str_update(Z_ARRVAL(all_node_data), SKYWALKING_DISTRIBUTED_TRACEIDS, sizeof(SKYWALKING_DISTRIBUTED_TRACEIDS) - 1, &distributed_trace_ids_array);
	zend_update_property(skywalking_ce_entry, this_ptr, ZEND_STRL("_allNodeData"), &all_node_data);
	/**
     * 第一个span节点初始操作
     * 当前页
	 */

	char *page_url_and_peer;
	zval span_first_node_data, span_string_param, *z_span_node_data_struct, z_page_url_and_peer;
	z_span_node_data_struct = zend_read_property(skywalking_ce_entry, this_ptr, ZEND_STRL("_spanNodeDataStruct"), 1, NULL);

	array_init(&span_first_node_data);
	zend_hash_copy(Z_ARRVAL(span_first_node_data), Z_ARRVAL_P(z_span_node_data_struct), (copy_ctor_func_t) zval_add_ref);

	page_url_and_peer = get_page_url_and_peer();

	ZVAL_STRING(&z_page_url_and_peer, page_url_and_peer);

	zend_string_init(page_url_and_peer, sizeof(page_url_and_peer) - 1, 0);
	zval z_span_id;
	zval first_millisecond;
	char *l_f_millisecond = get_millisecond();

	ZVAL_LONG(&first_millisecond, zend_atol(l_f_millisecond, strlen(l_f_millisecond)));

	ZVAL_LONG(&z_span_id, generate_span_id(this_ptr));
	zend_hash_str_update(Z_ARRVAL(span_first_node_data), SKYWALKING_SPAN_ID, sizeof(SKYWALKING_SPAN_ID) - 1, &z_span_id);
	zend_hash_str_update(Z_ARRVAL(span_first_node_data), SKYWALKING_STARTTIME, sizeof(SKYWALKING_STARTTIME) - 1, &first_millisecond);
	zend_hash_str_update(Z_ARRVAL(span_first_node_data), SKYWALKING_SPAN_SERVER_URI, sizeof(SKYWALKING_SPAN_SERVER_URI) - 1, &z_page_url_and_peer);

	array_init(&span_string_param);

	add_assoc_string(&span_string_param, "span.layer", "http");
	add_assoc_string(&span_string_param, "component", "php-server");
	add_assoc_string(&span_string_param, "peer.host", get_ip());
	add_assoc_string(&span_string_param, "url", page_url_and_peer);
	add_assoc_string(&span_string_param, "span.kind", "server");
	zend_hash_str_update(Z_ARRVAL(span_first_node_data), SKYWALKING_SPAN_STRING_PARAM, sizeof(SKYWALKING_SPAN_STRING_PARAM) - 1, &span_string_param);

	zend_update_property(skywalking_ce_entry, this_ptr, ZEND_STRL("_spanFirstNodeData"), &span_first_node_data);

	set_sampling_rate(this_ptr, SKY_G(global_sampling_rate));
	zend_update_property_string(skywalking_ce_entry, this_ptr, ZEND_STRL("_logPath"), SKY_G(global_log_path));
}



/**
 * 获取接收到 SWTraceContext 的 header
 */
static zval *receive_SWHeader_from_caller(zval *this_ptr)
{
	zend_bool jit_initialization = PG(auto_globals_jit);
	zval *ret;
	zval *sw_header_info;
	sw_header_info = zend_read_property(skywalking_ce_entry, this_ptr, ZEND_STRL("_swHeaderInfo"), 1, NULL);
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
			zend_hash_str_update(Z_ARRVAL_P(sw_header_info),   sw_header_info_keys[num_key], strlen(sw_header_info_keys[num_key]), operand);
		} ZEND_HASH_FOREACH_END();
	}
	return sw_header_info;

}

static void set_sampling_rate(zval *this_ptr, double degrees){

	long percent_int;

	degrees = _php_math_round(degrees, 2, PHP_ROUND_HALF_UP);
	percent_int = (int)(degrees*100);
	if( percent_int <= 0){
		percent_int = 100;
	}
	if(percent_int > 10000){
		percent_int = 10000;
	}
	zend_update_property_long(skywalking_ce_entry, this_ptr, ZEND_STRL("_samplingRate"), percent_int);
}


static char *generate_trace_id(zval *this_ptr){
	zval *sw_header_info;
	zval *ret;
	char *make_trace_id_c, *_trace_id;
	_trace_id = (char *)emalloc(sizeof(char)*180);
	bzero(_trace_id, 180);
	if( (sw_header_info = zend_read_property(skywalking_ce_entry, this_ptr, ZEND_STRL("_swHeaderInfo"), 1, NULL)) != NULL){
		if ((ret = zend_hash_str_find(Z_ARRVAL_P(sw_header_info), "TraceId", sizeof("TraceId") - 1)) != NULL) {
			return Z_STRVAL_P(ret);
		}
	}
	ret = zend_read_property(skywalking_ce_entry, this_ptr, ZEND_STRL("_traceId"), 1, NULL);
	if(Z_TYPE_P(ret)  != IS_NULL){
		return Z_STRVAL_P(ret);
	}
	make_trace_id_c = make_trace_id();
	sprintf(_trace_id, "Segment.%s", make_trace_id_c);
	zend_update_property_string(skywalking_ce_entry, this_ptr, ZEND_STRL("_traceId"), _trace_id);
	efree(make_trace_id_c);
	return _trace_id;

}


static char *generate_distributed_trace_ids(zval *this_ptr){
	zval *sw_header_info;
	zval *ret;
	char *make_trace_id_c, *_distributed_trace_ids;
	_distributed_trace_ids  = (char *)emalloc(sizeof(char)*180);
	bzero(_distributed_trace_ids, 180);
	if( (sw_header_info = zend_read_property(skywalking_ce_entry, this_ptr, ZEND_STRL("_swHeaderInfo"), 1, NULL)) != NULL ){
		if ((ret = zend_hash_str_find(Z_ARRVAL_P(sw_header_info), "DistributedTraceIds", sizeof("DistributedTraceIds") - 1)) != NULL) {
			return Z_STRVAL_P(ret);
		}
	}

	if(ret = zend_read_property(skywalking_ce_entry, this_ptr, ZEND_STRL("_distributedTraceIds"), 1, NULL));
	if(Z_TYPE_P(ret)  != IS_NULL){
		return Z_STRVAL_P(ret);
	}
	make_trace_id_c = make_trace_id();
    sprintf(_distributed_trace_ids, "Trace.%s", make_trace_id_c);
	zend_update_property_string(skywalking_ce_entry, this_ptr,  ZEND_STRL("_distributedTraceIds"), _distributed_trace_ids);
	efree(make_trace_id_c);
	return _distributed_trace_ids;
}


static char *make_trace_id(){
	//生产唯一码毫秒时间戳.uuid.当前进程号PID.当前线程ID.当前线程生成的流水号.ip
	char *millisecond;
	char *uuid;
	char *ip;
	char *makeTraceId;
	zval *carrier = NULL;
	zval *server_addr,*local_addr;
    millisecond = get_millisecond();
    uuid  = uniqid();
   	makeTraceId = (char *)emalloc(sizeof(char)*180);
   	bzero(makeTraceId, 180);
   	if (strcasecmp("cli", sapi_module.name) == 0){
   		ip = "127.0.0.1";
   	}else{
   		carrier = &PG(http_globals)[TRACK_VARS_SERVER];
   		if ((server_addr = zend_hash_str_find(Z_ARRVAL_P(carrier), "SERVER_ADDR", sizeof("SERVER_ADDR") - 1)) != NULL){
   			ip = Z_STRVAL_P(server_addr);
   		}else {
   			local_addr = zend_hash_str_find(Z_ARRVAL_P(carrier), "LOCAL_ADDR", sizeof("LOCAL_ADDR") - 1);
   			ip = Z_STRVAL_P(local_addr);
   		}
   	}
    sprintf(makeTraceId, "%s.%s.%d.0.0.%s", millisecond, uuid, getpid(), ip);

    efree(millisecond);
    efree(uuid);
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

static char *uniqid(){
	char *uniqid;
	int sec, usec;
	struct timeval tv;
	uniqid = (char *)emalloc(sizeof(char)*20);
	bzero(uniqid, 20);
	gettimeofday((struct timeval *) &tv, (struct timezone *) NULL);
	sec = (int) tv.tv_sec;
	usec = (int) (tv.tv_usec % 0x100000);

	/* The max value usec can have is 0xF423F, so we use only five hex
	 * digits for usecs.
	 */
	sprintf(uniqid, "%08x%05x",  sec, usec);

	return uniqid;
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



static char *get_ip(){

	zval *server_addr;
	char *ip, *ip_x;
	zval ip_z;
	zval *carrier = NULL;
	ip_x = (char *)emalloc(sizeof(char)*20);
	ip  = (char *)emalloc(sizeof(char)*100);

	bzero(ip_x, 20);
	bzero(ip, 100);
	carrier = &PG(http_globals)[TRACK_VARS_SERVER];
	if (strcasecmp("cli", sapi_module.name) == 0){
   		strcpy(ip, "127.0.0.1");
   	}else if(server_addr = zend_hash_str_find(Z_ARRVAL_P(carrier), SKY_G(global_header_client_ip_name), sizeof(SKY_G(global_header_client_ip_name)) - 1)){
		ip = Z_STRVAL_P(server_addr);
	}else if(server_addr = zend_hash_str_find(Z_ARRVAL_P(carrier), "HTTP_X_FORWARDED_FOR",  sizeof("HTTP_X_FORWARDED_FOR") - 1)){
		ip = Z_STRVAL_P(server_addr);
	}else if(server_addr = zend_hash_str_find(Z_ARRVAL_P(carrier), "HTTP_CLIENT_IP",  sizeof("HTTP_CLIENT_IP") - 1)){
		ip = Z_STRVAL_P(server_addr);
	}else if(server_addr = zend_hash_str_find(Z_ARRVAL_P(carrier), "REMOTE_ADDR",  sizeof("REMOTE_ADDR") - 1)){
		ip = Z_STRVAL_P(server_addr);
	}

	if(strstr(ip, ",") != NULL){
		array_init(&ip_z);
		zend_string *delim = zend_string_init(ZEND_STRL(","), 0);

		php_explode(delim, zend_string_init(ip, strlen(ip), 0), &ip_z, 10);

		ip_x = Z_STRVAL_P(zend_hash_index_find(Z_ARRVAL_P(&ip_z), 0));
	}else{
		memcpy(ip_x, ip, strlen(ip));
	}
	efree(ip);
	return ip_x;
}

static char *build_SWheader_value(const char *peer_host, zval *this_ptr){

	char *trace_id, *app_code, *distributed_trace_ids,*SW_header;
	int is_sample_int = 0;
	long span_id;
	//zval SW_trace_context;
	SW_header = (char *)emalloc(sizeof(char)*250);
	trace_id = generate_trace_id(this_ptr);
	span_id = generate_span_id(this_ptr);
	app_code = Z_STRVAL_P(zend_read_property(skywalking_ce_entry, this_ptr, ZEND_STRL("_appCode"), 1, NULL));
	distributed_trace_ids = generate_distributed_trace_ids(this_ptr);
	if(is_sampling(this_ptr) == IS_TRUE){
		is_sample_int = 1;
	}
	sprintf(SW_header, "%s|%ld|%s|%s|%s|%d", trace_id, span_id, app_code, peer_host, distributed_trace_ids, is_sample_int);


	return SW_header;
}

static long generate_span_id(zval *this_ptr){
	long span_id;
	span_id = Z_LVAL_P(zend_read_property(skywalking_ce_entry, this_ptr, ZEND_STRL("_spanID"), 1, NULL));
	zend_update_property_long(skywalking_ce_entry, this_ptr,	ZEND_STRL("_spanID"), ++span_id);
	return span_id;
}

static zend_always_inline zend_uchar is_sampling(zval *this_ptr){

	zval *z_p_is_sampling , *sw_header_info,*z_h_is_sample, *sampling_rate;
	int is_sampling_int;

	zend_long rand_val;

	z_p_is_sampling = zend_read_property(skywalking_ce_entry, this_ptr, ZEND_STRL("_isSampling"), 1, NULL);
	if(ZVAL_IS_NULL(z_p_is_sampling)){
		is_sampling_int = 0;
		sw_header_info = zend_read_property(skywalking_ce_entry, this_ptr, ZEND_STRL("_swHeaderInfo"), 1, NULL);
		if (sw_header_info && zend_hash_num_elements(Z_ARRVAL_P(sw_header_info))){
			z_h_is_sample = zend_hash_str_find(Z_ARRVAL_P(sw_header_info), ZEND_STRL("IsSample"));
			if(z_h_is_sample && strcmp(Z_STRVAL_P(z_h_is_sample),"1") == 0 ){
				is_sampling_int = 1;
			}
		}else{
			sampling_rate = zend_read_property(skywalking_ce_entry, this_ptr, ZEND_STRL("_samplingRate"), 1, NULL);
			if(Z_LVAL_P(sampling_rate) < 10000){
#if PHP_VERSION_ID >= 71000
				rand_val = php_mt_rand_common(1, 10000);
#else
				if (!BG(mt_rand_is_seeded)) {
					php_mt_srand(GENERATE_SEED());
				}
				rand_val = (zend_long) (php_mt_rand() >> 1);
				RAND_RANGE(rand_val, 1, 10000, PHP_MT_RAND_MAX);
#endif
				if(Z_LVAL_P(sampling_rate) >= rand_val){
					is_sampling_int = 1;
				}
			}
		}
		zend_update_property_bool(skywalking_ce_entry, this_ptr, ZEND_STRL("_isSampling"), is_sampling_int);
	}
	return Z_TYPE_P(z_p_is_sampling);
}


static zval *set_span_nodes_data(zval *this_ptr, zval *node_data){
	zval *spans_node_data;
	spans_node_data = zend_read_property(skywalking_ce_entry, this_ptr, ZEND_STRL("_spansNodeData"), 1, NULL);
	zend_hash_next_index_insert(Z_ARRVAL_P(spans_node_data), node_data);
}

static int is_auto_open(){
	if((strcasecmp("cli", sapi_module.name) != 0) && (SG(sapi_headers).http_response_code != 200)){
		return 0;
	}
	if(SKY_G(global_auto_open)){
		return 1;
	}
	return 0;
}


/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(skywalking)
{
	ZEND_INIT_MODULE_GLOBALS(skywalking, php_skywalking_init_globals, NULL);
	date_register_classes(TSRMLS_C);
	REGISTER_INI_ENTRIES();
	/* If you have INI entries, uncomment these lines
	*/
	if(SKY_G(global_auto_open)){
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
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(skywalking)
{
#if defined(COMPILE_DL_SKYWALKING) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	if(is_auto_open()){
		zval *instance, rv = {{0}};
		if ((instance = sky_instance(&rv, SKY_G(global_app_code))) != NULL) {
			ZVAL_COPY(&sky_t_ptr, instance);
		}
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
		sky_finishAll();
	}
	is_send_curl_header = 0;
	//zend_hash_destroy(Z_ARRVAL(super_curl_header));
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
