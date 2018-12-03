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

#define PHP_SKYWALKING_VERSION "5.0.0" /* Replace with version number for your extension */

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

#define S_SEND_TYPE_CLOSE   (1<<0L)
#define S_SEND_TYPE_GRPC    (1<<1L)
#define S_SEND_TYPE_WRITE   (1<<2L)

#define USE_PARENT_TRACE_ID 1
#define DONOT_USE_PARENT_TRACE_ID 0


#define SHARED_MEMORY_KEY 428935192
#define SKYWALKING_SEGMENT "sg" //全部段节点
#define SKYWALKING_DISTRIBUTED_TRACEIDS "gt"//DistributedTraceIds
#define SKYWALKING_TRACE_SEGMENT_ID "ts"//本次请求id
#define SKYWALKING_APPLICATION_ID "ai"//app id
#define SKYWALKING_APPLICATION_INSTANCE_ID "ii"//实例id
#define SKYWALKING_FATHER_NODE_DATA "rs" //父节点数据
#define SKYWALKING_SPANS_NODE_DATA "ss" //span节点数据集合

#define SKYWALKING_PARENT_TRACE_SEGMENT_ID "ts" //本次请求id
#define SKYWALKING_PARENT_APPLICATION_ID "ai"//app id
#define SKYWALKING_PARENT_SPAN_ID "si"//spanid
#define SKYWALKING_PARENT_SERVICE_ID "vi"//
#define SKYWALKING_PARENT_SERVICE_NAME "vn"
#define SKYWALKING_NETWORK_ADDRESS_ID "ni"
#define SKYWALKING_NETWORK_ADDRESS "nn"
#define SKYWALKING_ENTRY_APPLICATION_INSTANCE_ID "ea"
#define SKYWALKING_ENTRY_SERVICE_ID "ei"
#define SKYWALKING_ENTRY_SERVICE_NAME "en"
#define SKYWALKING_REF_TYPE_VALUE "rv"

#define SKYWALKING_SPAN_ID "si" //SpanId
#define SKYWALKING_SPAN_TYPE_VALUE "tv"
#define SKYWALKING_SPAN_LAYER_VALUE "lv"
#define SKYWALKING_FATHER_SPAN_ID "ps" //父节点传过来的SpanId
#define SKYWALKING_STARTTIME "st" //开始时间
#define SKYWALKING_ENDTIME "et"  //结束时间
#define SKYWALKING_COMPONENT_ID "ci"
#define SKYWALKING_COMPONENT_NAME "cn"
#define SKYWALKING_OPERATION_NAME_ID "oi"
#define SKYWALKING_OPERATION_NAME "on"// Span 的服务URI
#define SKYWALKING_PEER_ID "pi"
#define SKYWALKING_PEER "pn"
#define SKYWALKING_IS_ERROR "ie"
#define SKYWALKING_TAGS "to"
#define SKYWALKING_LOGS "lo"

#define SKYWALKING_KEY "k"
#define SKYWALKING_VALUE "v"

#define SKYWALKING_TIME "ti"
#define SKYWALKING_LOG_DATA "ld"



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

static void *write_log_text(char *text);
static char *sky_json_encode(zval *parameter);
static char *build_SWheader_value(const char *peer_host);
static zval receive_SWHeader_from_caller();
static char *get_millisecond();
static char *uniqid();
static char *generate_sw3(zend_long span_id, zend_string *peer_host, zend_string *operation_name);
static void make_trace_id();
static zend_always_inline zend_uchar is_sampling();
static char *generate_trace_id();
static char *get_ip();
static char *get_page_request_uri();
static char *get_page_url_and_peer();
static long generate_span_id();
static void *write_log( char *text);
static zval *set_span_nodes_data(zval *node_data);
static void request_init();
static long sky_array_unshift(zval *stack, zval *var);
static void set_sampling_rate(double degrees);
static void set_span_param_of_curl(char *peer_host);
static void make_span_curl_header(char *peer_host, zval *headers);
static void start_span(zval *this_pr);
static int is_auto_open();

void accel_sky_curl_init(INTERNAL_FUNCTION_PARAMETERS);
void accel_sky_curl_setopt(INTERNAL_FUNCTION_PARAMETERS);
void sky_curl_exec_handler(INTERNAL_FUNCTION_PARAMETERS);
void list_poll(comList* myroot);

static void *sky_flush_all();
static zval *get_first_span();
static zval *get_spans();
static char* _get_current_machine_ip();
static int get_app_instance_id();
static int  get_app_id();
static char *generate_parent_info_from_header(char *header_name);
static void start_node_span_of_curl();
static char *generate_parent_trace_id();
static zval* generate_trace_id_for_array(int use_parent_tid, zval* z_trace_id);
static char* _entry_app_name();
static char *generate_distributed_trace_ids();
static int _entry_app_instance_id();
static void end_node_span_of_curl(zval *curl);
static void* send_grpc_param(zval *all_node_data);
static comList* create_com_list();
static long _entry_app_name_operation_id();
static long _parent_appname_operation_id();
static int sky_live_pthread(pthread_t tid);
static int add_com_list(comList *com_list, UniqueIdStruct* data);
static void (*orig_curl_init)(INTERNAL_FUNCTION_PARAMETERS) = NULL;
static void (*orig_curl_setopt)(INTERNAL_FUNCTION_PARAMETERS) = NULL;
static void (*orig_curl_exec)(INTERNAL_FUNCTION_PARAMETERS) = NULL;
/*
  	Declare any global variables you may need between the BEGIN
	and END macros here:
*/
ZEND_BEGIN_MODULE_GLOBALS(skywalking)
	char *global_log_path;
  char *global_header_client_ip_name;
  char *global_app_code;
  long global_send_type;
  zend_bool global_auto_open;
  double global_sampling_rate;
  char *global_app_grpc_trace;
  zval UpstreamSegment;
  zval context;
ZEND_END_MODULE_GLOBALS(skywalking)

extern ZEND_DECLARE_MODULE_GLOBALS(skywalking);

/* Always refer to the globals in your function as SKYWALKING_G(variable).
   You are encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/
#ifdef ZTS
#define SKYWALKING_G(v) TSRMG(skywalking_globals_id, zend_skywalking_globals *, v)
#else
#define SKYWALKING_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(skywalking, v)
#endif

#if defined(ZTS) && defined(COMPILE_DL_SKYWALKING)
ZEND_TSRMLS_CACHE_EXTERN()
#endif


#ifdef __unix


#ifdef __linux
#define SKY_OS_NAME "Linux"
#endif

#ifdef __sun
    #ifdef __sparc
#define SKY_OS_NAME "Sun SPARC"
    #else
#define SKY_OS_NAME "Sun X86"
    #endif
#endif

#ifdef _AIX
#define SKY_OS_NAME "AIX"
#endif
 

#else


#ifdef WINVER
#define SKY_OS_NAME "Windows"
#else
#define SKY_OS_NAME "Unknown"
#endif
 

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
