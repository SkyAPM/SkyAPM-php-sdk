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
 */


#include "sky_plugin_redis.h"
#include "sky_util_php.h"
#include "sky_utils.h"
#include "ext/standard/php_smart_string.h"

// strings
zif_handler origin_redis_append = NULL;
zif_handler origin_redis_decr = NULL;
zif_handler origin_redis_decrby = NULL;
zif_handler origin_redis_get = NULL;
zif_handler origin_redis_getdel = NULL;
zif_handler origin_redis_getex = NULL;
zif_handler origin_redis_getrange = NULL;
zif_handler origin_redis_getset = NULL;
zif_handler origin_redis_incr = NULL;
zif_handler origin_redis_incrby = NULL;
zif_handler origin_redis_incrbyfloat = NULL;
zif_handler origin_redis_lcs = NULL;
zif_handler origin_redis_mget = NULL;
zif_handler origin_redis_mset = NULL;
zif_handler origin_redis_msetnx = NULL;
zif_handler origin_redis_psetex = NULL;
zif_handler origin_redis_set = NULL;
zif_handler origin_redis_setex = NULL;
zif_handler origin_redis_setnx = NULL;
zif_handler origin_redis_setrange = NULL;
zif_handler origin_redis_strlen = NULL;
zif_handler origin_redis_substr = NULL;

void sky_plugin_redis_hooks() {
    zend_function *origin_function;
    REDIS_HOOK("redis", "append", origin_redis_append, sky_plugin_redis_append_handler)
    REDIS_HOOK("redis", "decr", origin_redis_decr, sky_plugin_redis_decr_handler)
    REDIS_HOOK("redis", "decrby", origin_redis_decrby, sky_plugin_redis_decrby_handler)
    REDIS_HOOK("redis", "get", origin_redis_get, sky_plugin_redis_get_handler)
    REDIS_HOOK("redis", "getdel", origin_redis_getdel, sky_plugin_redis_getdel_handler)
    REDIS_HOOK("redis", "getex", origin_redis_getex, sky_plugin_redis_getex_handler)
    REDIS_HOOK("redis", "getrange", origin_redis_getrange, sky_plugin_redis_getrange_handler)
    REDIS_HOOK("redis", "getset", origin_redis_getset, sky_plugin_redis_getset_handler)
    REDIS_HOOK("redis", "incr", origin_redis_incr, sky_plugin_redis_incr_handler)
    REDIS_HOOK("redis", "incrby", origin_redis_incrby, sky_plugin_redis_incrby_handler)
    REDIS_HOOK("redis", "incrbyfloat", origin_redis_incrbyfloat, sky_plugin_redis_incrbyfloat_handler)
    REDIS_HOOK("redis", "lcs", origin_redis_lcs, sky_plugin_redis_lcs_handler)
    REDIS_HOOK("redis", "mget", origin_redis_mget, sky_plugin_redis_mget_handler)
    REDIS_HOOK("redis", "mset", origin_redis_mset, sky_plugin_redis_mset_handler)
    REDIS_HOOK("redis", "msetnx", origin_redis_msetnx, sky_plugin_redis_msetnx_handler)
    REDIS_HOOK("redis", "psetex", origin_redis_psetex, sky_plugin_redis_psetex_handler)
    REDIS_HOOK("redis", "set", origin_redis_set, sky_plugin_redis_set_handler)
    REDIS_HOOK("redis", "setex", origin_redis_setex, sky_plugin_redis_setex_handler)
    REDIS_HOOK("redis", "setnx", origin_redis_setnx, sky_plugin_redis_setnx_handler)
    REDIS_HOOK("redis", "setrange", origin_redis_setrange, sky_plugin_redis_setrange_handler)
    REDIS_HOOK("redis", "strlen", origin_redis_strlen, sky_plugin_redis_strlen_handler)
    REDIS_HOOK("redis", "substr", origin_redis_substr, sky_plugin_redis_substr_handler)

}

int sky_plugin_redis_command(char **command, char *kw, char *fmt, ...) {
    smart_string cmd = {0};

    va_list ap;
    union resparg arg;
    size_t arglen;
    va_start(ap, fmt);

    smart_string_appendl(&cmd, kw, strlen(kw));

    while (*fmt) {
        switch (*fmt) {
            case 'm':
                arg.zv = va_arg(ap, zval*);
                if (Z_TYPE_P(arg.zv) == IS_ARRAY) {
                    HashTable *hash = Z_ARRVAL_P(arg.zv);
                    if (zend_hash_num_elements(hash) > 0) {
                        zend_ulong idx;
                        zend_string *zkey;
                        zval *zmem;
                        char buf[64];
                        size_t keylen;
                        ZEND_HASH_FOREACH_KEY_VAL(hash, idx, zkey, zmem) {
                            if (zkey) {
                                smart_string_appendl(&cmd, " ", strlen(" "));
                                smart_string_appendl(&cmd, ZSTR_VAL(zkey), ZSTR_LEN(zkey));
                            } else {
                                keylen = snprintf(buf, sizeof(buf), "%ld", (long)idx);
                                smart_string_appendl(&cmd, " ", strlen(" "));
                                smart_string_appendl(&cmd, buf, keylen);
                            }
                            if (Z_TYPE_P(arg.zv) == IS_STRING) {
                                smart_string_appendl(&cmd, " ", strlen(" "));
                                smart_string_appendl(&cmd, Z_STRVAL_P(zmem), Z_STRLEN_P(zmem));
                            }
                        } ZEND_HASH_FOREACH_END();
                    }
                }
                break;
            case 'a':
                arg.zv = va_arg(ap, zval*);
                if (Z_TYPE_P(arg.zv) == IS_ARRAY) {
                    HashTable *hash = Z_ARRVAL_P(arg.zv);
                    if (zend_hash_num_elements(hash) > 0) {
                        zval *z_ele;
                        ZEND_HASH_FOREACH_VAL(hash, z_ele) {
                            zend_string *zstr = zval_get_string(z_ele);
                                    smart_string_appendl(&cmd, " ", strlen(" "));
                                    smart_string_appendl(&cmd, ZSTR_VAL(zstr), ZSTR_LEN(zstr));
                            zend_string_release(zstr);
                        } ZEND_HASH_FOREACH_END();
                    }
                }
                break;
            case 's':
                arg.str = va_arg(ap, char*);
                arglen = va_arg(ap, size_t);
                smart_string_appendl(&cmd, " ", strlen(" "));
                smart_string_appendl(&cmd, arg.str, arglen);
                break;
            case 'k':
                arg.str = va_arg(ap, char*);
                arglen = va_arg(ap, size_t);
                smart_string_appendl(&cmd, " ", strlen(" "));
                smart_string_appendl(&cmd, arg.str, arglen);
                break;
            case 'v':
                arg.zv = va_arg(ap, zval*);
                if (arg.zv && Z_TYPE_P(arg.zv) == IS_STRING) {
                    smart_string_appendl(&cmd, " ", strlen(" "));
                    smart_string_appendl(&cmd, Z_STRVAL_P(arg.zv), Z_STRLEN_P(arg.zv));
                }
                break;
            case 'f':
            case 'F':
                arg.dval = va_arg(ap, double);
                char tmp[64], *p;
                int len;
                len = snprintf(tmp, sizeof(tmp), "%.17g", arg.dval);
                if ((p = strchr(tmp, ',')) != NULL) *p = '.';
                smart_string_appendl(&cmd, " ", strlen(" "));
                smart_string_appendl(&cmd, tmp, len);
                break;
            case 'i':
            case 'd':
                arg.ival = va_arg(ap, int);
                char int_buf[32];
                int int_len = snprintf(int_buf, sizeof(int_buf), "%d", arg.ival);
                smart_string_appendl(&cmd, " ", strlen(" "));
                smart_string_appendl(&cmd, int_buf, int_len);
                break;
            case 'l':
            case 'L':
                arg.lval = va_arg(ap, long);
                char long_buf[32];
                int long_len = snprintf(long_buf, sizeof(long_buf), "%ld", arg.lval);
                smart_string_appendl(&cmd, " ", strlen(" "));
                smart_string_appendl(&cmd, long_buf, long_len);
                break;
        }
        fmt++;
    }

    va_end(ap);

    smart_string_0(&cmd);
    *command = cmd.c;
    return cmd.len;
}

void sky_plugin_redis_append_handler(INTERNAL_FUNCTION_PARAMETERS) {
    REDIS_KV_PARSE("Redis", "APPEND")
    origin_redis_append(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    REDIS_SPAN_END
}

void sky_plugin_redis_decr_handler(INTERNAL_FUNCTION_PARAMETERS) {
    REDIS_INDECR_PARSE("Redis", "DECR")
    origin_redis_decr(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    REDIS_SPAN_END
}

void sky_plugin_redis_decrby_handler(INTERNAL_FUNCTION_PARAMETERS) {
    REDIS_KL_PARSE("Redis", "DECRBY")
    origin_redis_decrby(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    REDIS_SPAN_END
}

void sky_plugin_redis_get_handler(INTERNAL_FUNCTION_PARAMETERS) {
    REDIS_K_PARSE("Redis", "GET")
    origin_redis_get(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    REDIS_SPAN_END
}

void sky_plugin_redis_getdel_handler(INTERNAL_FUNCTION_PARAMETERS) {
    origin_redis_getdel(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void sky_plugin_redis_getex_handler(INTERNAL_FUNCTION_PARAMETERS) {
    origin_redis_getex(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void sky_plugin_redis_getrange_handler(INTERNAL_FUNCTION_PARAMETERS) {
    REDIS_KLL_PARSE("Redis", "GETRANGE")
    origin_redis_getrange(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    REDIS_SPAN_END
}

void sky_plugin_redis_getset_handler(INTERNAL_FUNCTION_PARAMETERS) {
    REDIS_KV_PARSE("Redis", "GETSET")
    origin_redis_getset(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    REDIS_SPAN_END
}

void sky_plugin_redis_incr_handler(INTERNAL_FUNCTION_PARAMETERS) {
    REDIS_INDECR_PARSE("Redis", "INCR")
    origin_redis_incr(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    REDIS_SPAN_END
}

void sky_plugin_redis_incrby_handler(INTERNAL_FUNCTION_PARAMETERS) {
    REDIS_KL_PARSE("Redis", "INCRBY")
    origin_redis_incrby(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    REDIS_SPAN_END
}

void sky_plugin_redis_incrbyfloat_handler(INTERNAL_FUNCTION_PARAMETERS) {
    REDIS_KF_PARSE("Redis", "INCRBYFLOAT")
    origin_redis_incrbyfloat(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    REDIS_SPAN_END
}

void sky_plugin_redis_lcs_handler(INTERNAL_FUNCTION_PARAMETERS) {
    origin_redis_lcs(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void sky_plugin_redis_mget_handler(INTERNAL_FUNCTION_PARAMETERS) {
    REDIS_A_PARSE("Redis", "MGET")
    origin_redis_mget(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    REDIS_SPAN_END
}

void sky_plugin_redis_mset_handler(INTERNAL_FUNCTION_PARAMETERS) {
    REDIS_M_PARSE("Redis", "MSET")
    origin_redis_mset(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    REDIS_SPAN_END
}

void sky_plugin_redis_msetnx_handler(INTERNAL_FUNCTION_PARAMETERS) {
    REDIS_M_PARSE("Redis", "MSETNX")
    origin_redis_msetnx(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    REDIS_SPAN_END
}

void sky_plugin_redis_psetex_handler(INTERNAL_FUNCTION_PARAMETERS) {
    REDIS_KLV_PARSE("Redis", "PSETEX")
    origin_redis_psetex(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    REDIS_SPAN_END
}

void sky_plugin_redis_set_handler(INTERNAL_FUNCTION_PARAMETERS) {
    REDIS_SET_PARSE("Redis", "SET")
    origin_redis_set(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    REDIS_SPAN_END
}

void sky_plugin_redis_setex_handler(INTERNAL_FUNCTION_PARAMETERS) {
    REDIS_KLV_PARSE("Redis", "SETEX")
    origin_redis_setex(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    REDIS_SPAN_END
}

void sky_plugin_redis_setnx_handler(INTERNAL_FUNCTION_PARAMETERS) {
    REDIS_KV_PARSE("Redis", "SETNX")
    origin_redis_setnx(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    REDIS_SPAN_END
}

void sky_plugin_redis_setrange_handler(INTERNAL_FUNCTION_PARAMETERS) {
    REDIS_KDS_PARSE("Redis", "SETRANGE")
    origin_redis_setrange(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    REDIS_SPAN_END
}

void sky_plugin_redis_strlen_handler(INTERNAL_FUNCTION_PARAMETERS) {
    REDIS_K_PARSE("Redis", "STRLEN")
    origin_redis_strlen(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    REDIS_SPAN_END
}

void sky_plugin_redis_substr_handler(INTERNAL_FUNCTION_PARAMETERS) {
    origin_redis_substr(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

//std::unordered_map<std::string, redis_cmd_cb> commands = {
//        // connection
//        {"SELECT",      sky_plugin_redis_select_cmd},
//        {"ECHO",        sky_plugin_redis_key_cmd},
//        {"PING",        sky_plugin_redis_pure_cmd},
//        {"PIPELINE",    sky_plugin_redis_pure_cmd},
//
//        // server
//        // @todo
//
//        // strings
//        {"APPEND",      sky_plugin_redis_key_value_cmd},
//        {"BITCOUNT",    sky_plugin_redis_bit_count_cmd},
//        {"BITOP",       sky_plugin_redis_uncertain_keys_cmd},
//        {"BITPOS",      sky_plugin_redis_bit_pos_cmd},
//        {"DECR",        sky_plugin_redis_key_cmd},
//        {"DECRBY",      sky_plugin_redis_key_int_cmd},
//        {"GET",         sky_plugin_redis_key_cmd},
//        {"GETBIT",      sky_plugin_redis_key_int_cmd},
//        {"GETRANGE",    sky_plugin_redis_get_range_cmd},
//        {"GETSET",      sky_plugin_redis_key_value_cmd},
//        {"INCR",        sky_plugin_redis_key_cmd},
//        {"INCRBY",      sky_plugin_redis_key_int_cmd},
//        {"INCRBYFLOAT", sky_plugin_redis_key_float_cmd},
//        {"MGET",        sky_plugin_redis_multi_key_cmd},
//        {"GETMULTIPLE", sky_plugin_redis_multi_key_cmd},
//        {"MSET",        sky_plugin_redis_multi_key_value_cmd},
//        {"MSETNX",      sky_plugin_redis_multi_key_value_cmd},
//        {"SET",         sky_plugin_redis_set_cmd},
//        {"SETBIT",      sky_plugin_redis_set_bit_cmd},
//        {"SETEX",       sky_plugin_redis_setex_cmd},
//        {"PSETEX",      sky_plugin_redis_psetex_cmd},
//        {"SETNX",       sky_plugin_redis_key_value_cmd},
//        {"SETRANGE",    sky_plugin_redis_set_range_cmd},
//        {"STRLEN",      sky_plugin_redis_key_cmd},
//
//        // keys
//        {"DEL",         sky_plugin_redis_uncertain_keys_cmd},
//        {"DELETE",      sky_plugin_redis_uncertain_keys_cmd},
//        {"UNLINK",      sky_plugin_redis_uncertain_keys_cmd},
//        {"DUMP",        sky_plugin_redis_todo_cmd},
//        {"EXISTS",      sky_plugin_redis_uncertain_keys_cmd},
//        {"EXPIRE",      sky_plugin_redis_key_ttl_cmd},
//        {"SETTIMEOUT",  sky_plugin_redis_key_ttl_cmd},
//        {"PEXPIRE",     sky_plugin_redis_key_ttl_cmd},
//        {"EXPIREAT",    sky_plugin_redis_key_ttl_cmd},
//        {"PEXPIREAT",   sky_plugin_redis_key_ttl_cmd},
//        {"KEYS",        sky_plugin_redis_todo_cmd},
//        {"GETKEYS",     sky_plugin_redis_todo_cmd},
//        {"SCAN",        sky_plugin_redis_todo_cmd},
//        {"MIGRATE",     sky_plugin_redis_todo_cmd},
//        {"MOVE",        sky_plugin_redis_todo_cmd},
//        {"OBJECT",      sky_plugin_redis_todo_cmd},
//        {"PERSIST",     sky_plugin_redis_todo_cmd},
//        {"RANDOMKEY",   sky_plugin_redis_todo_cmd},
//        {"RENAME",      sky_plugin_redis_todo_cmd},
//        {"RENAMEKEY",   sky_plugin_redis_todo_cmd},
//        {"RENAMENX",    sky_plugin_redis_todo_cmd},
//        {"TYPE",        sky_plugin_redis_todo_cmd},
//        {"SORT",        sky_plugin_redis_todo_cmd},
//        {"TTL",         sky_plugin_redis_todo_cmd},
//        {"PTTL",        sky_plugin_redis_todo_cmd},
//        {"RESTORE",     sky_plugin_redis_todo_cmd},
//
//        // hashes
//        {"HDEL",        sky_plugin_redis_todo_cmd},
//        {"HEXISTS",     sky_plugin_redis_todo_cmd},
//        {"HGET",        sky_plugin_redis_todo_cmd},
//        {"HGETALL",     sky_plugin_redis_todo_cmd},
//        {"HINCRBY",     sky_plugin_redis_todo_cmd},
//        {"HINCRBYFLOAT",sky_plugin_redis_todo_cmd},
//        {"HKEYS",       sky_plugin_redis_todo_cmd},
//        {"HLEN",        sky_plugin_redis_todo_cmd},
//        {"HMGET",       sky_plugin_redis_todo_cmd},
//        {"HMSET",       sky_plugin_redis_todo_cmd},
//        {"HSET",        sky_plugin_redis_todo_cmd},
//        {"HSETNX",      sky_plugin_redis_todo_cmd},
//        {"HVALS",       sky_plugin_redis_todo_cmd},
//        {"HSCAN",       sky_plugin_redis_todo_cmd},
//        {"HSTRLEN",     sky_plugin_redis_todo_cmd},
//
//        // lists
//        {"BLPOP",       sky_plugin_redis_todo_cmd},
//        {"BRPOP",       sky_plugin_redis_todo_cmd},
//        {"BRPOPLPUSH",  sky_plugin_redis_todo_cmd},
//        {"LINDEX",      sky_plugin_redis_todo_cmd},
//        {"LGET",        sky_plugin_redis_todo_cmd},
//        {"LINSERT",     sky_plugin_redis_todo_cmd},
//        {"LLEN",        sky_plugin_redis_todo_cmd},
//        {"LSIZE",       sky_plugin_redis_todo_cmd},
//        {"LPOP",        sky_plugin_redis_todo_cmd},
//        {"LPUSH",       sky_plugin_redis_todo_cmd},
//        {"LPUSHX",      sky_plugin_redis_todo_cmd},
//        {"LRANGE",      sky_plugin_redis_todo_cmd},
//        {"LGETRANGE",   sky_plugin_redis_todo_cmd},
//        {"LREM",        sky_plugin_redis_todo_cmd},
//        {"LREMOVE",     sky_plugin_redis_todo_cmd},
//        {"LSET",        sky_plugin_redis_todo_cmd},
//        {"LTRIM",       sky_plugin_redis_todo_cmd},
//        {"LISTTRIM",    sky_plugin_redis_todo_cmd},
//        {"RPOP",        sky_plugin_redis_todo_cmd},
//        {"RPOPLPUSH",   sky_plugin_redis_todo_cmd},
//        {"RPUSH",       sky_plugin_redis_todo_cmd},
//        {"RPUSHX",      sky_plugin_redis_todo_cmd},
//
//        // sets
//        {"SADD",        sky_plugin_redis_sets_add_cmd},
//        {"SCARD",       sky_plugin_redis_todo_cmd},
//        {"SSIZE",       sky_plugin_redis_todo_cmd},
//        {"SDIFF",       sky_plugin_redis_todo_cmd},
//        {"SDIFFSTORE",  sky_plugin_redis_todo_cmd},
//        {"SINTER",      sky_plugin_redis_todo_cmd},
//        {"SINTERSTORE", sky_plugin_redis_todo_cmd},
//        {"SISMEMBER",   sky_plugin_redis_todo_cmd},
//        {"SCONTAINS",   sky_plugin_redis_todo_cmd},
//        {"SMEMBERS",    sky_plugin_redis_todo_cmd},
//        {"SGETMEMBERS", sky_plugin_redis_todo_cmd},
//        {"SMOVE",       sky_plugin_redis_todo_cmd},
//        {"SPOP",        sky_plugin_redis_todo_cmd},
//        {"SRANDMEMBER", sky_plugin_redis_todo_cmd},
//        {"SREM",        sky_plugin_redis_todo_cmd},
//        {"SREMOVE",     sky_plugin_redis_todo_cmd},
//        {"SUNION",      sky_plugin_redis_todo_cmd},
//        {"SUNIONSTORE", sky_plugin_redis_todo_cmd},
//        {"SSCAN",       sky_plugin_redis_todo_cmd},
//
//        // sorted sets
//        {"BZPOP",               sky_plugin_redis_todo_cmd},
//        {"ZADD",                sky_plugin_redis_todo_cmd},
//        {"ZCARD",               sky_plugin_redis_todo_cmd},
//        {"ZSIZE",               sky_plugin_redis_todo_cmd},
//        {"ZCOUNT",              sky_plugin_redis_todo_cmd},
//        {"ZINCRBY",             sky_plugin_redis_todo_cmd},
//        {"ZINTERSTORE",         sky_plugin_redis_todo_cmd},
//        {"ZINTER",              sky_plugin_redis_todo_cmd},
//        {"ZPOP",                sky_plugin_redis_todo_cmd},
//        {"ZRANGE",              sky_plugin_redis_todo_cmd},
//        {"ZRANGEBYSCORE",       sky_plugin_redis_todo_cmd},
//        {"ZREVRANGEBYSCORE",    sky_plugin_redis_todo_cmd},
//        {"ZRANGEBYLEX",         sky_plugin_redis_todo_cmd},
//        {"ZRANK",               sky_plugin_redis_todo_cmd},
//        {"ZREVRANK",            sky_plugin_redis_todo_cmd},
//        {"ZREM",                sky_plugin_redis_todo_cmd},
//        {"ZDLETE",              sky_plugin_redis_todo_cmd},
//        {"ZREMOVE",             sky_plugin_redis_todo_cmd},
//        {"ZREMRANGEBYRANK",     sky_plugin_redis_todo_cmd},
//        {"ZDELETERANGEBYRANK",  sky_plugin_redis_todo_cmd},
//        {"ZREMRANGEBYSCORE",    sky_plugin_redis_todo_cmd},
//        {"ZDELETERANGEBYSCORE", sky_plugin_redis_todo_cmd},
//        {"ZREMOVERANGEBYSCORE", sky_plugin_redis_todo_cmd},
//        {"ZREVRANGE",           sky_plugin_redis_todo_cmd},
//        {"ZSCORE",              sky_plugin_redis_todo_cmd},
//        {"ZUNIONSTORE",         sky_plugin_redis_todo_cmd},
//        {"ZUNION",              sky_plugin_redis_todo_cmd},
//        {"ZSCAN",               sky_plugin_redis_todo_cmd},
//
//        // hyperLogLogs
//        {"PFADD",               sky_plugin_redis_todo_cmd},
//        {"PFCOUNT",             sky_plugin_redis_todo_cmd},
//        {"PFMERGE",             sky_plugin_redis_todo_cmd},
//
//        // geocoding
//        {"GEOADD",              sky_plugin_redis_todo_cmd},
//        {"GEOHASH",             sky_plugin_redis_todo_cmd},
//        {"GEOPOS",              sky_plugin_redis_todo_cmd},
//        {"GEODIST",             sky_plugin_redis_todo_cmd},
//        {"GEORADIUS",           sky_plugin_redis_todo_cmd},
//        {"GEORADIUSBYMEMBER",   sky_plugin_redis_todo_cmd},
//
//        // streams
//        {"XACK",        sky_plugin_redis_todo_cmd},
//        {"XADD",        sky_plugin_redis_todo_cmd},
//        {"XCLAIM",      sky_plugin_redis_todo_cmd},
//        {"XDEL",        sky_plugin_redis_todo_cmd},
//        {"XGROUP",      sky_plugin_redis_todo_cmd},
//        {"XINFO",       sky_plugin_redis_todo_cmd},
//        {"XLEN",        sky_plugin_redis_todo_cmd},
//        {"XPENDING",    sky_plugin_redis_todo_cmd},
//        {"XRANGE",      sky_plugin_redis_todo_cmd},
//        {"XREAD",       sky_plugin_redis_todo_cmd},
//        {"XREADGROUP",  sky_plugin_redis_todo_cmd},
//        {"XREVRANGE",   sky_plugin_redis_todo_cmd},
//        {"XTRIM",       sky_plugin_redis_todo_cmd},
//
//        // pub/sub
//        {"PSUBSCRIBE",  sky_plugin_redis_todo_cmd},
//        {"PUBLISH",     sky_plugin_redis_todo_cmd},
//        {"SUBSCRIBE",   sky_plugin_redis_todo_cmd},
//        {"PUBSUB",      sky_plugin_redis_todo_cmd},
//
//
//        // generic
//        {"RAWCOMMAND",  sky_plugin_redis_todo_cmd},
//
//
//        // transactions
//        {"MULTI",       sky_plugin_redis_pure_cmd},
//        {"EXEC",        sky_plugin_redis_pure_cmd},
//        {"DISCARD",     sky_plugin_redis_pure_cmd},
//        {"WATCH",       sky_plugin_redis_todo_cmd},
//        {"UNWATCH",     sky_plugin_redis_todo_cmd},
//
//
//        // scripting
//        {"EVAL",            sky_plugin_redis_eval_cmd},
//        {"EVALSHA",         sky_plugin_redis_eval_cmd},
//        {"SCRIPT",          sky_plugin_redis_todo_cmd},
//        {"CLIENT",          sky_plugin_redis_todo_cmd},
//        {"GETLASTERROR",    sky_plugin_redis_todo_cmd},
//        {"CLEARLASTERROR",  sky_plugin_redis_todo_cmd},
//
//};
//
//SkyCoreSpan *sky_plugin_redis(zend_execute_data *execute_data, const std::string &class_name, const std::string &function_name) {
//
//    std::string cmd = function_name;
//    std::transform(function_name.begin(), function_name.end(), cmd.begin(), ::toupper);
//    if (commands.count(cmd) > 0) {
//        auto *segment = sky_get_segment(execute_data, -1);
//        if (segment != nullptr) {
//            auto *span = segment->createSpan(SkyCoreSpanType::Exit, SkyCoreSpanLayer::Cache, 7);
//            span->setOperationName(class_name + "->" + function_name);
//            span->addTag("db.type", "redis");
//
//            // peer
//            auto peer = sky_plugin_redis_peer(execute_data);
//            if (!peer.empty()) {
//                span->setPeer(peer);
//            }
//
//            span->addTag("redis.command", commands[cmd](execute_data, cmd));
//            return span;
//        }
//    }
//
//    return nullptr;
//}
//
//std::string sky_plugin_redis_peer(zend_execute_data *execute_data) {
//    zval *command = &(execute_data->This);
//    zval host;
//    zval port;
//#if PHP_VERSION_ID < 80000
//    zend_call_method(command, Z_OBJCE_P(command), nullptr, ZEND_STRL("gethost"), &host, 0, nullptr, nullptr);
//    zend_call_method(command, Z_OBJCE_P(command), nullptr, ZEND_STRL("getport"), &port, 0, nullptr, nullptr);
//#else
//    zend_call_method(Z_OBJ_P(command), Z_OBJCE_P(command), nullptr, ZEND_STRL("gethost"), &host, 0, nullptr, nullptr);
//    zend_call_method(Z_OBJ_P(command), Z_OBJCE_P(command), nullptr, ZEND_STRL("getport"), &port, 0, nullptr, nullptr);
//#endif
//
//    if (!Z_ISUNDEF(host) && !Z_ISUNDEF(port) && Z_TYPE(host) == IS_STRING) {
//        std::string peer(Z_STRVAL(host));
//
//        if (Z_TYPE(port) == IS_LONG) {
//            peer += ":" + std::to_string(Z_LVAL(port));
//        } else if (Z_TYPE(port) == IS_STRING) {
//            peer += ":" + std::string(Z_STRVAL(port));
//        } else {
//            peer += ":6379";
//        }
//
//        zval_dtor(&host);
//        zval_dtor(&port);
//
//        return peer;
//    }
//
//    return "";
//}
//
//std::string sky_plugin_redis_key_cmd(zend_execute_data *execute_data, std::string cmd) {
//    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
//    if (arg_count == 1) {
//        zval *key = ZEND_CALL_ARG(execute_data, 1);
//        if (Z_TYPE_P(key) == IS_STRING) {
//            return cmd + " " + std::string(Z_STRVAL_P(key));
//        }
//    }
//
//    return "";
//}
//
//std::string sky_plugin_redis_select_cmd(zend_execute_data *execute_data, std::string cmd) {
//    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
//    if (arg_count == 1) {
//        zval *db = ZEND_CALL_ARG(execute_data, 1);
//        if (Z_TYPE_P(db) == IS_LONG) {
//            return cmd + " " + std::to_string(Z_LVAL_P(db));
//        }
//    }
//    return "";
//}
//
//std::string sky_plugin_redis_key_value_cmd(zend_execute_data *execute_data, std::string cmd) {
//    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
//    if (arg_count == 2) {
//        zval *key = ZEND_CALL_ARG(execute_data, 1);
//        zval *value = ZEND_CALL_ARG(execute_data, 2);
//        if (Z_TYPE_P(key) == IS_STRING && Z_TYPE_P(value) == IS_STRING) {
//            return cmd + " " + std::string(Z_STRVAL_P(key)) + " " + std::string(Z_STRVAL_P(value));
//        }
//    }
//    return "";
//}
//
//std::string sky_plugin_redis_set_cmd(zend_execute_data *execute_data, std::string cmd) {
//    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
//    if (arg_count == 2) {
//        return sky_plugin_redis_key_value_cmd(execute_data, cmd);
//    }
//    if (arg_count == 3) {
//        zval *key = ZEND_CALL_ARG(execute_data, 1);
//        zval *value = ZEND_CALL_ARG(execute_data, 2);
//        zval *optional = ZEND_CALL_ARG(execute_data, 3);
//
//        if (Z_TYPE_P(key) == IS_STRING && Z_TYPE_P(value) == IS_STRING) {
//            cmd += " " + std::string(Z_STRVAL_P(key)) + " " + std::string(Z_STRVAL_P(value));
//
//            if (Z_TYPE_P(optional) == IS_LONG) {
//                cmd += " " + std::to_string(Z_LVAL_P(optional));
//            }
//
//            if (Z_TYPE_P(optional) == IS_ARRAY) {
//                zend_ulong nk;
//                zend_string *sk;
//                zval *val;
//                ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(optional), nk, sk, val) {
//                    if (sk == NULL && Z_TYPE_P(val) == IS_STRING) {
//                        cmd += " " + std::string(Z_STRVAL_P(val));
//                    } else if (Z_TYPE_P(val) == IS_LONG) {
//                        cmd += " " + std::string(ZSTR_VAL(sk)) + " " + std::to_string(Z_LVAL_P(val));
//                    }
//                } ZEND_HASH_FOREACH_END();
//            }
//            return cmd;
//        }
//    }
//    return "";
//}
//
//std::string sky_plugin_redis_setex_cmd(zend_execute_data *execute_data, std::string cmd) {
//    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
//    if (arg_count == 3) {
//        zval *key = ZEND_CALL_ARG(execute_data, 1);
//        zval *ttl = ZEND_CALL_ARG(execute_data, 2);
//        zval *val = ZEND_CALL_ARG(execute_data, 3);
//
//        if (Z_TYPE_P(key) == IS_STRING && Z_TYPE_P(ttl) == IS_LONG && Z_TYPE_P(val) == IS_STRING) {
//            return cmd + " " + std::string(Z_STRVAL_P(key)) + " " + std::to_string(Z_LVAL_P(ttl)) + " " + std::string(Z_STRVAL_P(val));
//        }
//    }
//    return "";
//}
//
//std::string sky_plugin_redis_multi_key_value_cmd(zend_execute_data *execute_data, std::string cmd) {
//    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
//    if (arg_count == 1) {
//        zval *key_values = ZEND_CALL_ARG(execute_data, 1);
//        if (Z_TYPE_P(key_values) == IS_ARRAY) {
//            zend_ulong nk;
//            zend_string *sk;
//            zval *val;
//            ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(key_values), nk, sk, val) {
//                if (sk != NULL && Z_TYPE_P(val) == IS_STRING) {
//                    cmd += " " + std::string(ZSTR_VAL(sk)) + " " + std::string(Z_STRVAL_P(val));
//                }
//            } ZEND_HASH_FOREACH_END();
//            return cmd;
//        }
//    }
//    return "";
//}
//
//std::string sky_plugin_redis_psetex_cmd(zend_execute_data *execute_data, std::string cmd) {
//    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
//    if (arg_count == 3) {
//        zval *key = ZEND_CALL_ARG(execute_data, 1);
//        zval *ttl = ZEND_CALL_ARG(execute_data, 2);
//        zval *val = ZEND_CALL_ARG(execute_data, 3);
//        if (Z_TYPE_P(key) == IS_STRING && Z_TYPE_P(ttl) == IS_LONG && Z_TYPE_P(val) == IS_STRING) {
//            return cmd + " " + std::string(Z_STRVAL_P(key)) + " " + std::to_string(Z_LVAL_P(ttl)) + " " + std::string(Z_STRVAL_P(val));
//        }
//    }
//    return "";
//}
//
//std::string sky_plugin_redis_key_int_cmd(zend_execute_data *execute_data, std::string cmd) {
//    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
//    if (arg_count == 2) {
//        zval *key = ZEND_CALL_ARG(execute_data, 1);
//        zval *delta = ZEND_CALL_ARG(execute_data, 2);
//        if (Z_TYPE_P(key) == IS_STRING && Z_TYPE_P(delta) == IS_LONG) {
//            return cmd + " " + std::string(Z_STRVAL_P(key)) + " " + std::to_string(Z_LVAL_P(delta));
//        }
//    }
//    return "";
//}
//
//std::string sky_plugin_redis_get_range_cmd(zend_execute_data *execute_data, std::string cmd) {
//    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
//    if (arg_count == 3) {
//        zval *key = ZEND_CALL_ARG(execute_data, 1);
//        zval *start = ZEND_CALL_ARG(execute_data, 2);
//        zval *end = ZEND_CALL_ARG(execute_data, 3);
//        if (Z_TYPE_P(key) == IS_STRING && Z_TYPE_P(start) == IS_LONG && Z_TYPE_P(end) == IS_LONG) {
//            return cmd + " " + std::string(Z_STRVAL_P(key)) + " " + std::to_string(Z_LVAL_P(start)) + " " + std::to_string(Z_LVAL_P(end));
//        }
//    }
//    return "";
//}
//
//std::string sky_plugin_redis_set_bit_cmd(zend_execute_data *execute_data, std::string cmd) {
//    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
//    if (arg_count == 3) {
//        zval *key = ZEND_CALL_ARG(execute_data, 1);
//        zval *offset = ZEND_CALL_ARG(execute_data, 2);
//        zval *val = ZEND_CALL_ARG(execute_data, 3);
//        if (Z_TYPE_P(key) == IS_STRING && Z_TYPE_P(offset) == IS_LONG && (Z_TYPE_P(val) == IS_LONG || Z_TYPE_P(val) == IS_TRUE || Z_TYPE_P(val) == IS_FALSE)) {
//            cmd += " " + std::string(Z_STRVAL_P(key)) + " " + std::to_string(Z_LVAL_P(offset));
//            if (Z_TYPE_P(val) == IS_LONG) {
//                cmd += " " + std::to_string(Z_LVAL_P(val));
//            } else if (Z_TYPE_P(val) == IS_TRUE) {
//                cmd += " 1";
//            } else if (Z_TYPE_P(val) == IS_FALSE) {
//                cmd += " 0";
//            }
//            return cmd;
//        }
//    }
//    return "";
//}
//
//std::string sky_plugin_redis_set_range_cmd(zend_execute_data *execute_data, std::string cmd) {
//    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
//    if (arg_count == 3) {
//        zval *key = ZEND_CALL_ARG(execute_data, 1);
//        zval *offset = ZEND_CALL_ARG(execute_data, 2);
//        zval *value = ZEND_CALL_ARG(execute_data, 3);
//        if (Z_TYPE_P(key) == IS_STRING && Z_TYPE_P(offset) == IS_LONG && Z_TYPE_P(value) == IS_STRING) {
//            return cmd + " " + std::string(Z_STRVAL_P(key)) + " " + std::to_string(Z_LVAL_P(offset)) + " " + std::string(Z_STRVAL_P(value));
//        }
//    }
//    return "";
//}
//
//std::string sky_plugin_redis_key_float_cmd(zend_execute_data *execute_data, std::string cmd) {
//    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
//    if (arg_count == 2) {
//        zval *key = ZEND_CALL_ARG(execute_data, 1);
//        zval *delta = ZEND_CALL_ARG(execute_data, 2);
//        if (Z_TYPE_P(key) == IS_STRING && Z_TYPE_P(delta) == IS_DOUBLE) {
//            return cmd + " " + std::string(Z_STRVAL_P(key)) + " " + std::to_string(Z_DVAL_P(delta));
//        }
//    }
//    return "";
//}
//
//std::string sky_plugin_redis_multi_key_cmd(zend_execute_data *execute_data, std::string cmd) {
//    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
//    if (arg_count == 1) {
//        zval *keys = ZEND_CALL_ARG(execute_data, 1);
//        if (Z_TYPE_P(keys) == IS_ARRAY) {
//            zend_ulong index;
//            zval *key;
//            ZEND_HASH_FOREACH_NUM_KEY_VAL(Z_ARRVAL_P(keys), index, key) {
//                if (Z_TYPE_P(key) == IS_STRING) {
//                    cmd += " " + std::string(Z_STRVAL_P(key));
//                }
//            } ZEND_HASH_FOREACH_END();
//            return cmd;
//        }
//    }
//    return "";
//}
//
//std::string sky_plugin_redis_key_ttl_cmd(zend_execute_data *execute_data, std::string cmd) {
//    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
//    if (arg_count == 2) {
//        zval *key = ZEND_CALL_ARG(execute_data, 1);
//        zval *ttl = ZEND_CALL_ARG(execute_data, 2);
//        if (Z_TYPE_P(key) == IS_STRING && Z_TYPE_P(ttl) == IS_LONG) {
//            return cmd + " " + std::string(Z_STRVAL_P(key)) + " " + std::to_string(Z_LVAL_P(ttl));
//        }
//    }
//    return "";
//}
//
//std::string sky_plugin_redis_uncertain_keys_cmd(zend_execute_data *execute_data, std::string cmd) {
//    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
//
//    if (arg_count == 1) {
//        zval *keys = ZEND_CALL_ARG(execute_data, 1);
//        if (Z_TYPE_P(keys) == IS_STRING) {
//            return cmd + " " + std::string(Z_STRVAL_P(keys));
//        }
//        if (Z_TYPE_P(keys) == IS_ARRAY) {
//            zend_ulong index;
//            zval *key;
//            ZEND_HASH_FOREACH_NUM_KEY_VAL(Z_ARRVAL_P(keys), index, key) {
//                if (Z_TYPE_P(key) == IS_STRING) {
//                    cmd += " " + std::string(Z_STRVAL_P(key));
//                }
//            } ZEND_HASH_FOREACH_END();
//            return cmd;
//        }
//    }
//
//    if (arg_count > 1) {
//        uint32_t i = 0;
//        for (i = 1; i <= arg_count; i++) {
//            zval *key = ZEND_CALL_ARG(execute_data, i);
//            if (Z_TYPE_P(key) == IS_STRING) {
//                cmd += " " + std::string(Z_STRVAL_P(key));
//            }
//        }
//        return cmd;
//    }
//
//    return "";
//}
//
//std::string sky_plugin_redis_todo_cmd(zend_execute_data *execute_data, std::string cmd) {
//    return "";
//}
//
//std::string sky_plugin_redis_pure_cmd(zend_execute_data *execute_data, std::string cmd) {
//    return "";
//}
//
//std::string sky_plugin_redis_bit_count_cmd(zend_execute_data *execute_data, std::string cmd) {
//    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
//
//    if (arg_count >= 1) {
//        for (uint32_t i = 1; i <= 3; ++i) {
//            if (i <= arg_count) {
//                zval *value = ZEND_CALL_ARG(execute_data, i);
//                if (Z_TYPE_P(value) == IS_LONG) {
//                    cmd += " " + std::to_string(Z_LVAL_P(value));
//
//                    if (arg_count == 2) {
//                        cmd += " -1";
//                    }
//
//                } else if (Z_TYPE_P(value) == IS_STRING) {
//                    cmd += " " + std::string(Z_STRVAL_P(value));
//                }
//            }
//        }
//        return cmd;
//    }
//    return "";
//}
//
//std::string sky_plugin_redis_bit_pos_cmd(zend_execute_data *execute_data, std::string cmd) {
//    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
//    if (arg_count >= 2) {
//        zval *key = ZEND_CALL_ARG(execute_data, 1);
//        zval *bit = ZEND_CALL_ARG(execute_data, 2);
//        if (Z_TYPE_P(key) == IS_STRING && Z_TYPE_P(bit) == IS_LONG) {
//            cmd += " " + std::string(Z_STRVAL_P(key)) + " " + std::to_string(Z_LVAL_P(bit));
//            if (arg_count >= 3) {
//                zval *start = ZEND_CALL_ARG(execute_data, 3);
//                if (Z_TYPE_P(start) == IS_LONG) {
//                    cmd += " " + std::to_string(Z_LVAL_P(start));
//                }
//            }
//            if (arg_count >= 4) {
//                zval *end = ZEND_CALL_ARG(execute_data, 4);
//                if (Z_TYPE_P(end) == IS_LONG) {
//                    cmd += " " + std::to_string(Z_LVAL_P(end));
//                }
//            }
//            return cmd;
//        }
//    }
//    return "";
//}
//
//std::string sky_plugin_redis_eval_cmd(zend_execute_data *execute_data, std::string cmd) {
//    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
//    if (arg_count >= 1) {
//        zval *script = ZEND_CALL_ARG(execute_data, 1);
//        if (Z_TYPE_P(script) == IS_STRING) {
//            cmd += " \"" + std::string(Z_STRVAL_P(script)) + "\"";
//            if (arg_count >= 3) {
//                zval *num_keys = ZEND_CALL_ARG(execute_data, 3);
//                if (Z_TYPE_P(num_keys) == IS_LONG) {
//                    cmd += " " + std::to_string(Z_LVAL_P(num_keys));
//                } else {
//                    cmd += " 0";
//                }
//            } else {
//                cmd += " 0";
//            }
//            if (arg_count >= 2) {
//                zval *args = ZEND_CALL_ARG(execute_data, 2);
//                if (Z_TYPE_P(args) == IS_ARRAY) {
//                    zend_ulong index;
//                    zval *arg;
//                    ZEND_HASH_FOREACH_NUM_KEY_VAL(Z_ARRVAL_P(args), index, arg) {
//                        if (Z_TYPE_P(arg) == IS_STRING) {
//                            cmd += " " + std::string(Z_STRVAL_P(arg));
//                        }
//                        if (Z_TYPE_P(arg) == IS_LONG) {
//                            cmd += " " + std::to_string(Z_LVAL_P(arg));
//                        }
//                        if (Z_TYPE_P(arg) == IS_DOUBLE) {
//                            cmd += " " + std::to_string(Z_DVAL_P(arg));
//                        }
//                    } ZEND_HASH_FOREACH_END();
//                }
//            }
//            return cmd;
//        }
//    }
//    return "";
//}
//
//std::string sky_plugin_redis_sets_add_cmd(zend_execute_data *execute_data, std::string cmd) {
//    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
//    if (arg_count >= 2) {
//        zval *key = ZEND_CALL_ARG(execute_data, 1);
//        if (Z_TYPE_P(key) == IS_STRING) {
//            cmd = cmd + " " + std::string(Z_STRVAL_P(key));
//        }
//        for (uint32_t i = 2; i <= arg_count; ++i) {
//            if (i <= arg_count) {
//                zval *value = ZEND_CALL_ARG(execute_data, i);
//                if (Z_TYPE_P(value) == IS_LONG) {
//                    cmd += " " + std::to_string(Z_LVAL_P(value));
//                } else if (Z_TYPE_P(value) == IS_STRING) {
//                    cmd += " " + std::string(Z_STRVAL_P(value));
//                }
//            }
//        }
//        return cmd;
//    }
//    return "";
//}
