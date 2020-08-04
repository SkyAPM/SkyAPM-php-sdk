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

#include "sky_execute.h"

#include "php.h"
#include "php_skywalking.h"

void (*ori_execute_ex)(zend_execute_data *execute_data) = NULL;

void (*ori_execute_internal)(zend_execute_data *execute_data, zval *return_value) = NULL;

ZEND_API void sky_execute_ex(zend_execute_data *execute_data) {
    if (SKYWALKING_G(segment) == NULL) {
        ori_execute_ex(execute_data);
        return;
    }

    ori_execute_ex(execute_data);
}

ZEND_API void sky_execute_internal(zend_execute_data *execute_data, zval *return_value) {
    if (SKYWALKING_G(segment) == NULL) {
        if (ori_execute_internal) {
            ori_execute_internal(execute_data, return_value);
        } else {
            execute_internal(execute_data, return_value);
        }
        return;
    }

    if (ori_execute_internal) {
        ori_execute_internal(execute_data, return_value);
    } else {
        execute_internal(execute_data, return_value);
    }
}