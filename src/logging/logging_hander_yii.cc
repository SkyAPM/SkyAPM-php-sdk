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
#include "src/logging/logging_hander_yii.h"
#include "segment.h"
#include "sky_utils.h"
#include <src/logging/logging_data.h>

static LogData* _log_message_parse(zval *pDest, zend_execute_data *arg, std::string yii_target_name);

enum YII_LOGGING_LEVEL {
    ERROR= 0x01,
    WARNING= 0x02,
    INFO= 0x04,
    TRACE= 0x08,
    PROFILE = 0x40,
    LEVEL_PROFILE_BEGIN= 0x50,
    LEVEL_PROFILE_END= 0x60,
    UNKNOWN
};
YiiLoggingHander::YiiLoggingHander(char *yii_target_name):_yii_target_name(yii_target_name){};

void YiiLoggingHander::before_parse(zend_execute_data *execute_data, std::vector<LogData *> &log_datas){};
bool YiiLoggingHander::is_support(zend_execute_data *execute_data, char *class_name, char *function_name, bool is_internal) {
    return class_name && function_name 
            && strcmp(class_name, _yii_target_name) == 0 
            && strcmp(function_name, "export") == 0;
};

void YiiLoggingHander::after_parse(zend_execute_data *execute_data, std::vector<LogData *> &log_datas, zval *return_value) {
    zval *message = sky_read_property(&(execute_data->This), "messages", 1);
    zend_array *messages = Z_ARRVAL_P(message);
    const std::vector<LogData *> log_dataaa;
    int count = zend_array_count(messages);
    for(int i = 0; i < count; i++) {
        LogData *log_data = _log_message_parse(zend_hash_index_find(messages, i), execute_data, _yii_target_name);
        if (log_data) {
            log_datas.push_back(log_data);
        }
    }
};

LogData* _log_message_parse(zval *pDest, zend_execute_data *execute_data, std::string yii_target_name) {
    /**
     * @var array logged messages. This property is managed by [[log()]] and [[flush()]].
     * Each log message is of the following structure:
     *
     * ```
     * [
     *   [0] => message (mixed, can be a string or some complex data, such as an exception object)
     *   [1] => level (integer)
     *   [2] => category (string)
     *   [3] => timestamp (float, obtained by microtime(true))
     *   [4] => traces (array, debug backtrace, contains the application code call stacks)
     *   [5] => memory usage in bytes (int, obtained by memory_get_usage()), available since version 2.0.11.
     * ]
     * ```
     */
    auto *segment = sky_get_segment(execute_data, -1);
    zend_array *messages = Z_ARRVAL_P(pDest);
    zval* message = zend_hash_index_find(messages, 0);
    zval* level_val = zend_hash_index_find(messages, 1);
    if (Z_TYPE_P(message) != IS_STRING) {
        return nullptr;
    }
    char *content = ZSTR_VAL(Z_STR_P(message));
    int level_num = Z_LVAL_P(level_val);
    std::string level_tag;
    switch(level_num) {
        case ERROR:
            level_tag = "ERROR";
            break;
        case WARNING:
            level_tag = "WARNING";
            break;
        case INFO:
            level_tag = "INFO";
            break;
        case PROFILE:
            level_tag = "PROFILE";
            break;
        case LEVEL_PROFILE_BEGIN:
            level_tag = "LEVEL_PROFILE_BEGIN";
            break;
        case LEVEL_PROFILE_END:
            level_tag = "LEVEL_PROFILE_END";
            break;
        default:
            level_tag = "UNKNOWN";
    }
    auto time_now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    LogData *log_data = new LogData();
    std::string trace_id;
    if (segment) {
        trace_id = segment->getTraceId();
        log_data->addTraceContext(trace_id,segment->getSegmentId(), 0);
    }
    log_data->setTimestamp(time_now.count());
    log_data->addBody(TEXT, content);
    log_data->addTag("level", level_tag);
    log_data->addTag("logger", yii_target_name);
    return log_data;
};