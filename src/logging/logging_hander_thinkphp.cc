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
#include "src/logging/logging_hander_thinkphp.h"

ThinkphpLoggingHander::ThinkphpLoggingHander(char* target_name):_target_name(target_name){};

bool ThinkphpLoggingHander::is_support(zend_execute_data *execute_data, char *class_name, char *function_name, bool is_after) {
    return class_name && function_name 
            && strcmp(class_name, _target_name) == 0 
            && strcmp(function_name, "save") == 0;
}
void ThinkphpLoggingHander::after_parse(zend_execute_data *execute_data, std::vector<LogData *> &log_datas, zval *return_value){

}
void ThinkphpLoggingHander::before_parse(zend_execute_data *execute_data, std::vector<LogData *> &log_datas) {
    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
    if (!arg_count) {
        return;
    }

    zval *log_info = ZEND_CALL_ARG(execute_data, 1);
    zend_string *level;
    zval *msg, *item;
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
    ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(log_info), level, msg) {
        if (Z_TYPE_P(msg) == IS_ARRAY) {
            ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(msg), item) {
                LogData *log_data = new LogData();
                log_data->addTraceContext(trace_id,segment_id, 0);
                log_data->setTimestamp(time_now);
                log_data->addTag("level", ZSTR_VAL(level));
                log_data->addBody(TEXT, ZSTR_VAL(Z_STR_P(item)));
                log_datas.push_back(log_data);
            }ZEND_HASH_FOREACH_END();
        } else if (Z_TYPE_P(msg) == IS_STRING) {
            LogData *log_data = new LogData();
            log_data->addTraceContext(trace_id,segment_id, 0);
            log_data->setTimestamp(time_now);
            log_data->addTag("level", ZSTR_VAL(level));
            log_data->addBody(TEXT, ZSTR_VAL(Z_STR_P(msg)));
            log_datas.push_back(log_data);
        }
    }ZEND_HASH_FOREACH_END();
}