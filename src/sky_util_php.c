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

#include "sky_util_php.h"

void *sky_util_find_obj_func(const char *obj, const char *name) {
    zend_class_entry *ce = zend_hash_str_find_ptr(CG(class_table), obj, strlen(obj));
    if (ce != NULL) {
        return zend_hash_str_find_ptr(&ce->function_table, name, strlen(name));
    }
    return NULL;
}

void *sky_util_find_func(const char *name) {
    return zend_hash_str_find_ptr(CG(function_table), name, strlen(name));
}

void sky_util_json_raw(smart_string *dist, char *key, char *value, size_t value_len) {
    sky_util_json_key(dist, key);
    smart_string_appendl(dist, value, value_len);
}

void sky_util_json_raw_ex(smart_string *dist, char *key, char *value, size_t value_len) {
    sky_util_json_raw(dist, key, value, value_len);
    smart_string_appendc(dist, ',');
}

void sky_util_json_key(smart_string *dist, char *key) {
    smart_string_appendc(dist, '"');
    smart_string_appendl(dist, key, strlen(key));
    smart_string_appendc(dist, '"');
    smart_string_appendc(dist, ':');
}

void sky_util_json_int(smart_string *dist, char *key, zend_long num) {
    sky_util_json_key(dist, key);
    smart_string_append_long(dist, num);
}

void sky_util_json_int_ex(smart_string *dist, char *key, zend_long num) {
    sky_util_json_int(dist, key, num);
    smart_string_appendc(dist, ',');
}

void sky_util_json_str(smart_string *dist, char *key, char *value, size_t value_len) {
    sky_util_json_key(dist, key);
    smart_string_appendc(dist, '"');
    smart_string_appendl(dist, value, value_len);
    smart_string_appendc(dist, '"');
}

void sky_util_json_str_ex(smart_string *dist, char *key, char *value, size_t value_len) {
    sky_util_json_str(dist, key, value, value_len);
    smart_string_appendc(dist, ',');
}

void sky_util_json_bool(smart_string *dist, char *key, bool value) {
    sky_util_json_key(dist, key);
    if (value) {
        smart_string_appendl(dist, "true", 4);
    } else {
        smart_string_appendl(dist, "false", 5);
    }
}

void sky_util_json_bool_ex(smart_string *dist, char *key, bool value) {
    sky_util_json_bool(dist, key, value);
    smart_string_appendc(dist, ',');
}