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

#ifndef SKYWALKING_SKY_UTILS_H
#define SKYWALKING_SKY_UTILS_H

#include "php_skywalking.h"

#include <string>

#define SKY_IS_OBJECT(p) p != nullptr && Z_TYPE_P(p) == IS_OBJECT

bool starts_with(const char *pre, const char *str);

std::string get_page_request_uri();

std::string get_page_request_peer();

zval *sky_read_property(zval *obj, const char *property, int parent);

std::string sky_get_class_name(zval *obj);

#endif //SKYWALKING_SKY_UTILS_H
