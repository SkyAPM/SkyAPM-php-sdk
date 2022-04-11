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


#ifndef SKYWALKING_SKY_PLUGIN_RABBIT_MQ_H
#define SKYWALKING_SKY_PLUGIN_RABBIT_MQ_H

#include "php_skywalking.h"
#include "sky_core_span.h"

SkyCoreSpan *sky_plugin_rabbit_mq(zend_execute_data *execute_data, const std::string &class_name, const std::string &function_name);


#endif //SKYWALKING_SKY_PLUGIN_RABBIT_MQ_H
