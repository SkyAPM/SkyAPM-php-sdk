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

#ifndef SKYWALKING_SRC_SKY_PLUGIN_MEMCACHED_H_
#define SKYWALKING_SRC_SKY_PLUGIN_MEMCACHED_H_

#include "php_skywalking.h"
#include "sky_utils.h"
#include <string>
#include <functional>
#include <map>
#include "sky_core_span.h"
#include "sky_core_segment.h"

SkyCoreSpan *sky_plugin_memcached(zend_execute_data *execute_data, const std::string &class_name, const std::string &function_name);

std::string sky_plugin_memcached_peer(zend_execute_data *execute_data);

std::string sky_plugin_memcached_key_cmd(zend_execute_data *execute_data, std::string cmd);
#endif //SKYWALKING_SRC_SKY_PLUGIN_MEMCACHED_H_
