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

#ifndef PHP_SKYWALKING_H
#define PHP_SKYWALKING_H

extern zend_module_entry skywalking_module_entry;
#define phpext_skywalking_ptr &skywalking_module_entry

#define PHP_SKYWALKING_VERSION "0.1.0" /* Replace with version number for your extension */

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

#ifdef ZTS 
#define SKY_G(v) TSRMG(skywalking_globals_id, zend_skywalking_globals *, v)
#else
#define SKY_G(v) (skywalking_globals.v)
#endif

#define SKYWALKING_TRACEID "ts"  //TraceId
#define SKYWALKING_STARTTIME "st"//开始时间
#define SKYWALKING_ENDTIME "et" //结束时间
#define SKYWALKING_APP_CODE "ac"// App Code
#define SKYWALKING_FATHER_NODE_DATA "rs"//父节点数据
#define SKYWALKING_SPANS_NODE_DATA "ss"//span节点数据集合
#define SKYWALKING_DISTRIBUTED_TRACEIDS "gt" //DistributedTraceIds
#define SKYWALKING_SPAN_ID "si"//SpanId
#define SKYWALKING_PEERHOST "ph"//PeerHost
#define SKYWALKING_FATHER_SPAN_ID "ps"//父节点传过来的SpanId
#define SKYWALKING_SPAN_SERVER_URI "on" // Span 的服务URI
#define SKYWALKING_SPAN_STRING_PARAM "ts" //Span 的字符串型参数
#define SKYWALKING_SPAN_BOOL_PARAM "tb" //Span 的字符串型参数
#define SKYWALKING_SPAN_INT_PARAM "ti" //Span 的字符串型参数
#define SKYWALKING_SPAN_LOG "lo"//Span 的日志


int ski_zval_key = 0;
#define SKY_MAKE_NULL_ARRAY_NAME(i_zval_key)  null_array_##i_zval_key

#define RAND_RANGE(__n, __min, __max, __tmax) \
    (__n) = ((__min) + (zend_long) ((double) ( (double) (__max) - (__min) + 1.0) * ((__n) / ((__tmax) + 1.0))))

#define PHP_MT_RAND_MAX ((zend_long) (0x7FFFFFFF)) /* (1<<31) - 1 */

#ifdef PHP_WIN32
#define GENERATE_SEED() (((zend_long) (time(0) * GetCurrentProcessId())) ^ ((zend_long) (1000000.0 * php_combined_lcg())))
#else
#define GENERATE_SEED() (((zend_long) (time(0) * getpid())) ^ ((zend_long) (1000000.0 * php_combined_lcg())))
#endif


void *SKY_ADD_ASSOC_ZVAL(zval *z, const char *k) {

    zval null_array;  
    array_init(&null_array);  
    add_assoc_zval(z, k, &null_array); 
}

void *SKY_UPDATE_PROPERTY(zend_class_entry *cls, zval *tp, const char *name, size_t name_length)  {
    zval null_array;  
    array_init(&null_array);  
    zend_update_property(cls, tp, name, name_length, &null_array);
} 
static char *sky_json_encode(zval *parameter);
static int _php_filter_validate_domain(char * domain, int len, zend_long flags) ;
static int _php_filter_validate_ipv4(char *str, size_t str_len, int *ip);
static int _php_filter_validate_ipv6(char *str, size_t str_len);
static int _php_filter_validate_url(char *value);
static zval *sky_instance(zval *this_ptr, const char *appCode);
static void date_register_classes(TSRMLS_D);
static void _init(const char *appCode, zval *this_ptr);
static char *build_SWheader_value(const char *peer_host, zval *this_ptr);
static zval *receive_SWHeader_from_caller(zval *this_ptr);
static char *get_millisecond();
static char *uniqid();
static char *make_trace_id();
static zend_always_inline zend_uchar is_sampling(zval *this_ptr);
static char *generate_trace_id(zval *this_ptr);
static char *get_ip();
static char *get_page_url_and_peer();
static long generate_span_id(zval *this_ptr);
static char *generate_distributed_trace_ids(zval *this_ptr);
static void *write_log(zval *this_ptr, char *text);
static zval *set_span_nodes_data(zval *this_ptr, zval *node_data);
static char *sky_finishAll();
static long sky_array_unshift(zval *stack, zval *var);
static void set_sampling_rate(zval *this_ptr, double degrees);
static void start_span_of_curl(char *peer_host, zval *headers, zval *this_pr);
static void set_span_param_of_curl(char *peer_host, zval *this_pr);
static void make_span_curl_header(char *peer_host, zval *headers, zval *this_pr);
static void start_span(zval *this_pr);
static int is_auto_open();
static void end_span_of_curl(zval *this_ptr,zval *curl);
void accel_sky_curl_init(INTERNAL_FUNCTION_PARAMETERS);
void accel_sky_curl_setopt(INTERNAL_FUNCTION_PARAMETERS);
void accel_sky_curl_exec(INTERNAL_FUNCTION_PARAMETERS);

/*
  	Declare any global variables you may need between the BEGIN
	and END macros here:
*/
ZEND_BEGIN_MODULE_GLOBALS(skywalking)
	char *global_log_path;
  char *global_header_client_ip_name;
  char *global_app_code;
  zend_bool global_auto_open;
  double global_sampling_rate;
ZEND_END_MODULE_GLOBALS(skywalking)

extern ZEND_DECLARE_MODULE_GLOBALS(skywalking);

/* Always refer to the globals in your function as SKYWALKING_G(variable).
   You are encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/
#define SKYWALKING_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(skywalking, v)

#if defined(ZTS) && defined(COMPILE_DL_SKYWALKING)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif	/* PHP_SKYWALKING_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
