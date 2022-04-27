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

#ifndef SKYWALKING_SKY_CORE_MODULE_H
#define SKYWALKING_SKY_CORE_MODULE_H

#include "php.h"
#include "pthread.h"

#define SKY_FIND_FUNC(name) zend_hash_str_find_ptr(CG(function_table), name, sizeof(name) - 1)

typedef struct skywalking_t {
    char service[1024];
    char serviceInstance[1024];
    char *connect;
} skywalking_t;

int sky_core_module_init(INIT_FUNC_ARGS);

static ZEND_RSRC_DTOR_FUNC(skywalking_dtor);

void sky_core_module_free();

void sky_core_request_init(zval *request, u_int64_t request_id);

void sky_core_request_free(zval *response, u_int64_t request_id);

#endif //SKYWALKING_SKY_CORE_MODULE_H
