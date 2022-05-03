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
#include "segment.h"
#include "sky_utils.h"
#include <src/logging/logging_data.h>
#include "src/logging/logging_hander_internal.h"

InternalLoggingHander::InternalLoggingHander(){};

bool InternalLoggingHander::is_support(zend_execute_data *execute_data, char *class_name, char *function_name, bool is_internal) {
    return function_name && is_internal && strcmp(function_name, "error_log") == 0;
}
void InternalLoggingHander::after_parse(zend_execute_data *execute_data, std::vector<LogData *> &log_datas, zval *return_value){

}
void InternalLoggingHander::before_parse(zend_execute_data *execute_data, std::vector<LogData *> &log_datas) {
    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
    if (arg_count < 1) {
        return;
    }
    zval *message = ZEND_CALL_ARG(execute_data, 1);
    auto time_now = getUnixTimeStamp();
    auto *segment = sky_get_segment(execute_data, -1);
    std::string trace_id;
    std::string segment_id;
    if (segment) {
        trace_id = segment->getTraceId();
        segment_id = segment->getSegmentId();
    } else {
        trace_id = random_generator_uuid();
        segment_id = random_generator_uuid();
    }
    LogData *log_data = new LogData();
    log_data->addTraceContext(trace_id,segment_id, 0);
    log_data->setTimestamp(time_now);
    log_data->addTag("level", "info");
    log_data->addTag("logger", "error_log");
    log_data->addBody(TEXT, ZSTR_VAL(Z_STR_P(message)));
    log_datas.push_back(log_data);
}