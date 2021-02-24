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

#ifndef SKYWALKING_SKY_MYSQLI_H
#define SKYWALKING_SKY_MYSQLI_H

#include "php_skywalking.h"
#include "span.h"
#include <string>
#include "ext/mysqli/php_mysqli_structs.h"

Span *sky_plugin_mysqli(zend_execute_data *execute_data, const std::string &class_name, const std::string &function_name);

void sky_plugin_mysqli_check_errors(zend_execute_data *execute_data, Span *span, int is_oop);

#endif //SKYWALKING_SKY_MYSQLI_H
