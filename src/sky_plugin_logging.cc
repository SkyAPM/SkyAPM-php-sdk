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
#include "sky_plugin_logging.h"
#include "php_skywalking.h"
#include "sky_log.h"
#include <boost/interprocess/ipc/message_queue.hpp>

extern struct service_info *s_info;
extern void (*ori_execute_ex)(zend_execute_data *execute_data);
extern void (*ori_execute_internal)(zend_execute_data *execute_data, zval *return_value);
bool _hander_each(zend_execute_data *execute_data, bool is_internal, zval *return_value);
std::vector<LoggingHander *> _logging_handers;

void register_logging_hander(LoggingHander *hander){
    _logging_handers.push_back(hander);
}

void _send_message_and_release(std::vector<LogData *> &log_datas){
    if (log_datas.empty()) {
        return;
    }
    boost::interprocess::message_queue mq(
        boost::interprocess::open_only,
        s_info->log_mq_name
    );

    for (auto log_data : log_datas) {
        log_data->setService(s_info->service);
        log_data->setServiceInstance(s_info->service_instance);
        skywalking_logging_report(log_data);
        delete log_data;
    }

    log_datas.clear();
    log_datas.shrink_to_fit();
};

bool sky_plugin_logging_exec(zend_execute_data *execute_data) {
    if (_logging_handers.empty() || strlen(s_info->service_instance) == 0) {
        return false;
    }
    return _hander_each(execute_data, false, nullptr);
}

bool sky_plugin_logging_internal_exec(zend_execute_data *execute_data, zval *return_value) {
    if (_logging_handers.empty() || strlen(s_info->service_instance) == 0) {
        return false;
    }
    return _hander_each(execute_data, true, return_value);
}

bool _hander_each(zend_execute_data *execute_data, bool is_internal, zval *return_value){
    zend_function *fn = execute_data->func;
    int is_class = fn->common.scope != nullptr && fn->common.scope->name != nullptr;
    char *class_name = is_class ? ZSTR_VAL(fn->common.scope->name) : nullptr;
    char *function_name = fn->common.function_name != nullptr ? ZSTR_VAL(fn->common.function_name) : nullptr;

    for (auto hander : _logging_handers) {
        if (!hander->is_support(execute_data, class_name, function_name, is_internal)) {
            continue;
        }

        std::vector<LogData *> logDatas;
        hander->before_parse(execute_data, logDatas);
        if (is_internal) {
            if (ori_execute_internal) {
                ori_execute_internal(execute_data, return_value);
            } else {
                execute_internal(execute_data, return_value);
            }
        }else {
            ori_execute_ex(execute_data);
        }
        hander->after_parse(execute_data, logDatas, return_value);
        _send_message_and_release(logDatas);
        return true;
    }
    return false;
}