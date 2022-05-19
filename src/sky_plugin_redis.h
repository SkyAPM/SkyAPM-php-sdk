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


#ifndef SKYWALKING_SKY_PLUGIN_REDIS_H
#define SKYWALKING_SKY_PLUGIN_REDIS_H

//#include "php_skywalking.h"
//#include "sky_utils.h"
//#include <string>
//#include <functional>
//#include <unordered_map>
//#include "sky_core_span.h"
//#include "sky_core_segment.h"
#include "php.h"


#define REDIS_HOOK(class, func, origin_handle, handle) \
    if ((origin_function = sky_util_find_obj_func(class, func)) != NULL) { \
        origin_handle = origin_function->internal_function.handler; \
        origin_function->internal_function.handler = handle; \
    }

#define REDIS_K_PARSE(obj, kw) \
    char *key; \
    size_t key_len; \
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &key, &key_len) == FAILURE) { \
        RETURN_FALSE; \
    } \
    char *cmd = NULL; \
    sky_plugin_redis_command(&cmd, kw, "k", key, key_len); \
    REDIS_SPAN(obj, cmd)

#define REDIS_A_PARSE(obj, kw) \
    zval *z_args; \
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "a", &z_args) == FAILURE) { \
        RETURN_FALSE; \
    } \
    char *cmd = NULL; \
    sky_plugin_redis_command(&cmd, kw, "a", z_args); \
    REDIS_SPAN(obj, cmd)

#define REDIS_M_PARSE(obj, kw) \
    zval *z_array; \
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "a", &z_array) == FAILURE) { \
        RETURN_FALSE; \
    } \
    char *cmd = NULL; \
    sky_plugin_redis_command(&cmd, kw, "m", z_array); \
    REDIS_SPAN(obj, cmd)

#define REDIS_KDS_PARSE(obj, kw) \
    char *key, *val; \
    size_t key_len, val_len; \
    zend_long lval; \
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sls", &key, &key_len, &lval, &val, &val_len) == FAILURE) { \
        RETURN_FALSE; \
    } \
    char *cmd = NULL; \
    sky_plugin_redis_command(&cmd, kw, "kds", key, key_len, (int)lval, val, val_len); \
    REDIS_SPAN(obj, cmd)

#define REDIS_KV_PARSE(obj, kw) \
    char *key; \
    size_t key_len; \
    zval *z_val; \
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sz", &key, &key_len, &z_val) == FAILURE) { \
        RETURN_FALSE; \
    } \
    char *cmd = NULL; \
    sky_plugin_redis_command(&cmd, kw, "kv", key, key_len, z_val); \
    REDIS_SPAN(obj, cmd)

#define REDIS_KF_PARSE(obj, kw) \
    char *key; \
    size_t key_len; \
    double val; \
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sd", &key, &key_len, &val) == FAILURE) { \
        RETURN_FALSE; \
    } \
    char *cmd = NULL; \
    sky_plugin_redis_command(&cmd, kw, "kf", key, key_len, val); \
    REDIS_SPAN(obj, cmd)

#define REDIS_KL_PARSE(obj, kw) \
    char *key; \
    size_t key_len; \
    zend_long lval; \
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sl", &key, &key_len, &lval) == FAILURE) { \
        RETURN_FALSE; \
    } \
    char *cmd = NULL; \
    sky_plugin_redis_command(&cmd, kw, "kl", key, key_len, lval); \
    REDIS_SPAN(obj, cmd)

#define REDIS_KLV_PARSE(obj, kw) \
    char *key; \
    size_t key_len; \
    zend_long expire; \
    zval *z_val; \
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "slz", &key, &key_len, &expire, &z_val) == FAILURE) { \
        RETURN_FALSE; \
    } \
    char *cmd = NULL; \
    sky_plugin_redis_command(&cmd, kw, "klv", key, key_len, expire, z_val); \
    REDIS_SPAN(obj, cmd)

#define REDIS_KLL_PARSE(obj, kw) \
    char *key; \
    size_t key_len; \
    zend_long val1, val2; \
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sll", &key, &key_len, &val1, &val2) == FAILURE) { \
        RETURN_FALSE; \
    } \
    char *cmd = NULL; \
    sky_plugin_redis_command(&cmd, kw, "kll", key, key_len, val1, val2); \
    REDIS_SPAN(obj, cmd)

#define REDIS_INDECR_PARSE(obj, kw) \
    char *key; \
    size_t key_len; \
    zend_long val = 1; \
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|l", &key, &key_len, &val) == FAILURE) { \
        RETURN_FALSE; \
    } \
    char *cmd = NULL; \
    if (val == 1) { \
       sky_plugin_redis_command(&cmd, kw, "k", key, key_len); \
    } else { \
        char nkw[32] = {0}; \
        strcpy(nkw, kw); \
        strcat(nkw, "BY"); \
        sky_plugin_redis_command(&cmd, kw, "kl", key, key_len, val); \
    } \
    REDIS_SPAN(obj, cmd)

#define REDIS_SET_PARSE(obj, kw) \
    char *key = NULL; \
    size_t key_len; \
    zval *z_value, *z_opts = NULL; \
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sz|z", &key, &key_len, &z_value, &z_opts) == FAILURE) { \
        RETURN_FALSE; \
    } \
    char *cmd = NULL; \
    sky_plugin_redis_command(&cmd, kw, "kv", key, key_len, z_value); \
    REDIS_SPAN(obj, cmd)

#define REDIS_SPAN(operation, command) \
    sky_core_span_t *span = sky_core_span_new(Exit, Cache, 7); \
    sky_core_span_add_tag(span, sky_core_tag_new("db.type", "redis")); \
    sky_core_span_add_tag(span, sky_core_tag_new("db.command", command)); \
    efree(command); \
    sky_core_span_set_operation_name(span, operation);

#define REDIS_SPAN_END \
    sky_core_span_set_end_time(span); \
    sky_core_segment_t *segment = sky_util_find_segment_idx(execute_data, -1); \
    if (segment != NULL) { \
        sky_core_segment_add_span(segment, span); \
    }


union resparg {
    char *str;
    zend_string *zstr;
    zval *zv;
    int ival;
    long lval;
    double dval;
};

void sky_plugin_redis_hooks();

int sky_plugin_redis_command(char **command, char *kw, char *fmt, ...);

// strings
void sky_plugin_redis_append_handler(INTERNAL_FUNCTION_PARAMETERS);

void sky_plugin_redis_decr_handler(INTERNAL_FUNCTION_PARAMETERS);

void sky_plugin_redis_decrby_handler(INTERNAL_FUNCTION_PARAMETERS);

void sky_plugin_redis_get_handler(INTERNAL_FUNCTION_PARAMETERS);

void sky_plugin_redis_getdel_handler(INTERNAL_FUNCTION_PARAMETERS);

void sky_plugin_redis_getex_handler(INTERNAL_FUNCTION_PARAMETERS);

void sky_plugin_redis_getrange_handler(INTERNAL_FUNCTION_PARAMETERS);

void sky_plugin_redis_getset_handler(INTERNAL_FUNCTION_PARAMETERS);

void sky_plugin_redis_incr_handler(INTERNAL_FUNCTION_PARAMETERS);

void sky_plugin_redis_incrby_handler(INTERNAL_FUNCTION_PARAMETERS);

void sky_plugin_redis_incrbyfloat_handler(INTERNAL_FUNCTION_PARAMETERS);

void sky_plugin_redis_lcs_handler(INTERNAL_FUNCTION_PARAMETERS);

void sky_plugin_redis_mget_handler(INTERNAL_FUNCTION_PARAMETERS);

void sky_plugin_redis_mset_handler(INTERNAL_FUNCTION_PARAMETERS);

void sky_plugin_redis_msetnx_handler(INTERNAL_FUNCTION_PARAMETERS);

void sky_plugin_redis_psetex_handler(INTERNAL_FUNCTION_PARAMETERS);

void sky_plugin_redis_set_handler(INTERNAL_FUNCTION_PARAMETERS);

void sky_plugin_redis_setex_handler(INTERNAL_FUNCTION_PARAMETERS);

void sky_plugin_redis_setnx_handler(INTERNAL_FUNCTION_PARAMETERS);

void sky_plugin_redis_setrange_handler(INTERNAL_FUNCTION_PARAMETERS);

void sky_plugin_redis_strlen_handler(INTERNAL_FUNCTION_PARAMETERS);

void sky_plugin_redis_substr_handler(INTERNAL_FUNCTION_PARAMETERS);

//
//typedef std::function<std::string(zend_execute_data *execute_data, std::string)> redis_cmd_cb;
//
//SkyCoreSpan *sky_plugin_redis(zend_execute_data *execute_data, const std::string &class_name, const std::string &function_name);
//
//std::string sky_plugin_redis_peer(zend_execute_data *execute_data);
//
//std::string sky_plugin_redis_key_cmd(zend_execute_data *execute_data, std::string cmd);
//
//std::string sky_plugin_redis_set_cmd(zend_execute_data *execute_data, std::string cmd);
//
//std::string sky_plugin_redis_setex_cmd(zend_execute_data *execute_data, std::string cmd);
//
//std::string sky_plugin_redis_multi_key_value_cmd(zend_execute_data *execute_data, std::string cmd);
//
//std::string sky_plugin_redis_key_int_cmd(zend_execute_data *execute_data, std::string cmd);
//
//std::string sky_plugin_redis_key_float_cmd(zend_execute_data *execute_data, std::string cmd);
//
//std::string sky_plugin_redis_get_range_cmd(zend_execute_data *execute_data, std::string cmd);
//
//std::string sky_plugin_redis_set_bit_cmd(zend_execute_data *execute_data, std::string cmd);
//
//std::string sky_plugin_redis_set_range_cmd(zend_execute_data *execute_data, std::string cmd);
//
//std::string sky_plugin_redis_psetex_cmd(zend_execute_data *execute_data, std::string cmd);
//
//std::string sky_plugin_redis_multi_key_cmd(zend_execute_data *execute_data, std::string cmd);
//
//std::string sky_plugin_redis_uncertain_keys_cmd(zend_execute_data *execute_data, std::string cmd);
//
//std::string sky_plugin_redis_key_ttl_cmd(zend_execute_data *execute_data, std::string cmd);
//
//std::string sky_plugin_redis_bit_count_cmd(zend_execute_data *execute_data, std::string cmd);
//
//std::string sky_plugin_redis_bit_pos_cmd(zend_execute_data *execute_data, std::string cmd);
//
//std::string sky_plugin_redis_key_value_cmd(zend_execute_data *execute_data, std::string cmd);
//
//std::string sky_plugin_redis_todo_cmd(zend_execute_data *execute_data, std::string cmd);
//
//std::string sky_plugin_redis_pure_cmd(zend_execute_data *execute_data, std::string cmd);
//
//std::string sky_plugin_redis_eval_cmd(zend_execute_data *execute_data, std::string cmd);
//
//std::string sky_plugin_redis_select_cmd(zend_execute_data *execute_data, std::string cmd);
//
//// sets
//std::string sky_plugin_redis_sets_add_cmd(zend_execute_data *execute_data, std::string cmd);

#endif // SKYWALKING_SKY_PLUGIN_REDIS_H
