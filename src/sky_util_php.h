/*
 * Copyright 2022 SkyAPM
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

#ifndef SKYWALKING_SKY_UTIL_PHP_H
#define SKYWALKING_SKY_UTIL_PHP_H

#include "php.h"
#include "stdbool.h"

#if PHP_VERSION_ID < 70200
typedef void (*zif_handler)(INTERNAL_FUNCTION_PARAMETERS);
#endif

#if PHP_VERSION_ID >= 70200

#include "zend_smart_string.h"

#else
#include "zend_smart_str.h"
#endif

#if PHP_VERSION_ID >= 70200
#define sky_util_smart_string smart_string
#define sky_util_smart_string_appendc(str, c) smart_string_appendc((str), (c))
#define sky_util_smart_string_appendl(str, src, len) smart_string_appendl(str, src, len)
#define sky_util_smart_string_append_long(str, val) smart_string_append_long(str, val)
#define sky_util_smart_string_0(str) smart_string_0(str)
#define sky_util_smart_string_to_char(str) str.c
#define sky_util_smart_string_len(str) str.len
#define sky_util_smart_string_free(str) smart_string_free(str);
#else
#define sky_util_smart_string smart_str
#define sky_util_smart_string_appendc(str, c) smart_str_appendc((str), (c))
#define sky_util_smart_string_appendl(str, src, len) smart_str_appendl(str, src, len)
#define sky_util_smart_string_append_long(str, val) smart_str_append_long(str, val)
#define sky_util_smart_string_0(str) smart_str_0(str)
#define sky_util_smart_string_to_char(str) ZSTR_VAL(str.s)
#define sky_util_smart_string_len(str) str.a
#define sky_util_smart_string_free(str) smart_str_free(str);
#endif

void *sky_util_find_obj_func(const char *obj, const char *name);

void *sky_util_find_func(const char *name);

void sky_util_json_raw(sky_util_smart_string *dist, char *key, char *value, size_t value_len);

void sky_util_json_raw_ex(sky_util_smart_string *dist, char *key, char *value, size_t value_len);

void sky_util_json_key(sky_util_smart_string *dist, char *key);

void sky_util_json_int(sky_util_smart_string *dist, char *key, zend_long num);

void sky_util_json_int_ex(sky_util_smart_string *dist, char *key, zend_long num);

void sky_util_json_str(sky_util_smart_string *dist, char *key, char *value, size_t value_len);

void sky_util_json_str_ex(sky_util_smart_string *dist, char *key, char *value, size_t value_len);

void sky_util_json_bool(sky_util_smart_string *dist, char *key, bool value);

void sky_util_json_bool_ex(sky_util_smart_string *dist, char *key, bool value);

#endif //SKYWALKING_SKY_UTIL_PHP_H
