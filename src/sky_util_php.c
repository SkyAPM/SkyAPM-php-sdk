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