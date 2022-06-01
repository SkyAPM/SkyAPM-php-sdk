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


#ifndef SKYWALKING_SKY_MODULE_H
#define SKYWALKING_SKY_MODULE_H

#include "php_skywalking.h"

#define SKY_OLD_FN(n) static_cast<zend_function *>(zend_hash_str_find_ptr(CG(function_table), n, sizeof(n) - 1))

void sky_module_init();

void sky_module_cleanup();

void sky_request_init(zval *request, uint64_t request_id);


#endif //SKYWALKING_SKY_MODULE_H
