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

#include "ext/standard/php_smart_string.h"

void *sky_util_find_obj_func(const char *obj, const char *name);

void *sky_util_find_func(const char *name);

void sky_util_json_raw(smart_string *dist, char *key, char *value, size_t value_len);

void sky_util_json_raw_ex(smart_string *dist, char *key, char *value, size_t value_len);

void sky_util_json_key(smart_string *dist, char *key);

void sky_util_json_int(smart_string *dist, char *key, zend_long num);

void sky_util_json_int_ex(smart_string *dist, char *key, zend_long num);

void sky_util_json_str(smart_string *dist, char *key, char *value, size_t value_len);

void sky_util_json_str_ex(smart_string *dist, char *key, char *value, size_t value_len);

void sky_util_json_bool(smart_string *dist, char *key, bool value);

void sky_util_json_bool_ex(smart_string *dist, char *key, bool value);

#endif //SKYWALKING_SKY_UTIL_PHP_H
