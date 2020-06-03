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

#define SKY_DEBUG 0
#define PHP_SKYWALKING_VERSION "3.3.1" /* Replace with version number for your extension */

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

#define REDIS_KEY_KEY "|dump|exists|expire|expireat|move|persist|pexpire|pexpireat|pttl|rename|renamenx|sort|ttl|type|"
#define REDIS_KEY_STRING "|append|bitcount|bitfield|decr|decrby|get|getbit|getrange|getset|incr|incrby|incrbyfloat|psetex|set|setbit|setex|setnx|setrange|strlen|"
#define REDIS_OPERATION_STRING "|bitop|"
#define REDIS_KEY_HASH "|hdel|hexists|hget|hgetall|hincrby|hincrbyfloat|hkeys|hlen|hmget|hmset|hscan|hset|hsetnx|hvals|hstrlen|"
#define REDIS_KEY_LIST "|lindex|linsert|llen|lpop|lpush|lpushx|lrange|lrem|lset|ltrim|rpop|rpush|rpushx|"
#define REDIS_KEY_SET "|sadd|scard|sismember|smembers|spop|srandmember|srem|sscan|"
#define REDIS_KEY_SORT "|zadd|zcard|zcount|zincrby|zrange|zrangebyscore|zrank|zrem|zremrangebyrank|zremrangebyscore|zrevrange|zrevrangebyscore|zrevrank|zscore|zscan|zrangebylex|zrevrangebylex|zremrangebylex|zlexcount|"
#define REDIS_KEY_HLL "|pfadd|watch|"
#define REDIS_KEY_GEO "|geoadd|geohash|geopos|geodist|georadius|georadiusbymember|"

#define MEMCACHED_KEY_STRING "|set|setbykey|setmulti|setmultibykey|add|addbykey|replace|replacebykey|append|appendbykey|prepend|prependbykey|cas|casbykey|get|getbykey|getmulti|getmultibykey|getallkeys|delete|deletebykey|deletemulti|deletemultibykey|increment|incrementbykey|decrement|decrementbykey|"
#define MEMCACHED_KEY_STATS "|getstats|"
#define MEMCACHED_KEY_OTHERS "|ispersistent|ispristine|flush|flushbuffers|getdelayed|getdelayedbykey|fetch|fetchall|addserver|addservers|getoption|getresultcode|setoption|setoptions|getserverbykey|getserverlist|resetserverlist|getversion|quit|setsaslauthdata|touch|touchbykey|"

#ifdef ZEND_ENABLE_ZVAL_LONG64
#define PRId3264 PRId64
#else
#define PRId3264 PRId32
#endif

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
    return NULL;
}

typedef struct ContextCarrier {
    zval primaryDistributedTraceId;
    zval traceSegmentId;
    zval spanId;
    zval parentServiceInstanceId;
    zval entryServiceInstanceId;
    zval peerHost;
    zval entryEndpointName;
    zval parentEndpointName;
}ContextCarrier;


static char *sky_json_encode(zval *parameter);
static long get_second();
static char *get_millisecond();
static char *generate_sw3(zend_long span_id, char *peer_host, char *operation_name);
static char *generate_sw6(zend_long span_id, char *peer_host);
static char *generate_sw8(zend_long span_id);
static void generate_context();
static char *get_page_request_uri();
static char *get_page_request_peer();
static void write_log( char *text);
static void request_init();
static void zval_b64_encode(zval *out, char *in);
static void zval_b64_decode(zval *out, char *in);
static char *sky_get_class_name(zval *obj);
static zval *sky_read_property(zval *obj, const char *property);
static char *sky_redis_fnamewall(const char *function_name);
static int sky_redis_opt_for_string_key(char *fnamewall);

static char *sky_memcached_fnamewall(const char *function_name);
static int sky_memcached_opt_for_string_key(char *fnamewall);

void sky_curl_exec_handler(INTERNAL_FUNCTION_PARAMETERS);
void sky_curl_setopt_handler(INTERNAL_FUNCTION_PARAMETERS);
void sky_curl_setopt_array_handler(INTERNAL_FUNCTION_PARAMETERS);
void sky_curl_close_handler(INTERNAL_FUNCTION_PARAMETERS);

static void sky_flush_all();
static zval *get_first_span();
static zval *get_spans();
static char* _get_current_machine_ip();
static void (*orig_curl_exec)(INTERNAL_FUNCTION_PARAMETERS) = NULL;
static void (*orig_curl_setopt)(INTERNAL_FUNCTION_PARAMETERS) = NULL;
static void (*orig_curl_setopt_array)(INTERNAL_FUNCTION_PARAMETERS) = NULL;
static void (*orig_curl_close)(INTERNAL_FUNCTION_PARAMETERS) = NULL;
/*
  	Declare any global variables you may need between the BEGIN
	and END macros here:
*/
ZEND_BEGIN_MODULE_GLOBALS(skywalking)
    char *sock_path;
    char *app_code;
    zend_bool enable;
    zval UpstreamSegment;
    zval context;
    zval curl_header;
    zval curl_header_send;
    int  version;
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
#ifdef __APPLE__
#define SKY_OS_NAME "Darwin"
#else
#define SKY_OS_NAME "Unknown"
#endif
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
