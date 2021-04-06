// contributor license agreements.  See the NOTICE file distributed with
// this work for additional information regarding copyright ownership.
// The ASF licenses this file to You under the Apache License, Version 2.0
// (the "License"); you may not use this file except in compliance with
// the License.  You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SKYWALKING_SKY_PLUGIN_REDIS_H
#define SKYWALKING_SKY_PLUGIN_REDIS_H

#include "php_skywalking.h"
#include "sky_utils.h"
#include <string>
#include <functional>
#include <map>
#include "span.h"
#include "segment.h"


typedef std::function<std::string(zend_execute_data *execute_data, std::string)> redis_cmd_cb;

Span *sky_plugin_redis(zend_execute_data *execute_data, const std::string &class_name, const std::string &function_name);

std::string sky_plugin_redis_peer(zend_execute_data *execute_data);

std::string sky_plugin_redis_key_cmd(zend_execute_data *execute_data, std::string cmd);

std::string sky_plugin_redis_set_cmd(zend_execute_data *execute_data, std::string cmd);

std::string sky_plugin_redis_setex_cmd(zend_execute_data *execute_data, std::string cmd);

std::string sky_plugin_redis_multi_key_cmd(zend_execute_data *execute_data, std::string cmd);

std::string sky_plugin_redis_uncertain_keys_cmd(zend_execute_data *execute_data, std::string cmd);

std::string sky_plugin_redis_key_ttl_cmd(zend_execute_data *execute_data, std::string cmd);

std::string sky_plugin_redis_bit_count_cmd(zend_execute_data *execute_data, std::string cmd);

std::string sky_plugin_redis_key_value_cmd(zend_execute_data *execute_data, std::string cmd);

std::string sky_plugin_redis_todo_cmd(zend_execute_data *execute_data, std::string cmd);


#endif // SKYWALKING_SKY_PLUGIN_REDIS_H
