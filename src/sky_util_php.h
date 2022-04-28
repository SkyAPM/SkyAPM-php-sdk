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

#if PHP_VERSION_ID >= 70200

#include "zend_smart_string.h"

#else
#include "zend_smart_str.h"
#endif

#if PHP_VERSION_ID >= 70200
#define sky_util_smart_string smart_string
#define sky_util_smart_string_appendl(str, src, len) smart_string_appendl(str, src, len)
#define sky_util_smart_string_0(str) smart_string_0(str)
#define sky_util_smart_string_to_char(str) str.c
#define sky_util_smart_string_len(str) str.len
#else
#define sky_util_smart_string smart_str
#define sky_util_smart_string_appendl(str, src, len) smart_str_appendl(str, src, len)
#define sky_util_smart_string_0(str) smart_str_0(str)
#define sky_util_smart_string_to_char(str) ZSTR_VAL(Z_STR_P(str.s))
#define sky_util_smart_string_len(str) str.a
#endif

void *sky_util_find_obj_func(const char *obj, const char *name);

#endif //SKYWALKING_SKY_UTIL_PHP_H
