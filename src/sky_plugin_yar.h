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

#ifndef SKYWALKING_SRC_SKY_PLUGIN_YAR_H_
#define SKYWALKING_SRC_SKY_PLUGIN_YAR_H_

#include "php_skywalking.h"
#include "sky_utils.h"
#include <string>
#include <functional>
#include <map>
#include "sky_core_span.h"
#include "sky_core_segment.h"
#define YAR_OPT_HEADER	(1<<4)

SkyCoreSpan *sky_plugin_yar_client(zend_execute_data *execute_data, const std::string &class_name, const std::string &function_name);

SkyCoreSpan *sky_plugin_yar_server(zend_execute_data *execute_data, const std::string &class_name, const std::string &function_name);

#endif //SKYWALKING_SRC_SKY_PLUGIN_YAR_H_
