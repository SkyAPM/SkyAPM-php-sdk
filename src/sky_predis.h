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

#ifndef SKYWALKING_SKY_PREDIS_H
#define SKYWALKING_SKY_PREDIS_H

#include "php_skywalking.h"

#include "sky_utils.h"
#include <string>
#include "span.h"

#define SKY_PREDIS_IS_PARAMETERS(p) SKY_IS_OBJECT(p) && sky_get_class_name(p) == "Predis\\Connection\\Parameters"

#define SKY_PREDIS_IS_STREAM_CONNECTION(c) SKY_IS_OBJECT(c) && sky_get_class_name(c) == "Predis\\Connection\\StreamConnection"


Span *sky_predis(zend_execute_data *execute_data, char *class_name, char *function_name);

static std::string sky_predis_peer(zend_execute_data *execute_data);

static std::string sky_predis_command(zval *id, zval *arguments);

#endif //SKYWALKING_SKY_PREDIS_H
