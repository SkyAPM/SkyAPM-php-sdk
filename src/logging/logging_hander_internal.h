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
#ifndef SKYWALKING_INTERNAL_LOGGING_HANDER_H
#define SKYWALKING_INTERNAL_LOGGING_HANDER_H
#include "src/logging/logging_data.h"
#include "src/logging/logging_common.h"
#include <vector>

class InternalLoggingHander : public LoggingHander {
    public:
        InternalLoggingHander();
        bool is_support(zend_execute_data *execute_data, char *class_name, char *function_name, bool is_internal);
        void after_parse(zend_execute_data *execute_data, std::vector<LogData *> &log_datas, zval *return_value);
        void before_parse(zend_execute_data *execute_data, std::vector<LogData *> &log_datas);
};

#endif
