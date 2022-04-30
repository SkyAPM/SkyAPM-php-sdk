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
#include "src/logging/logging_common.h"
#include <boost/interprocess/ipc/message_queue.hpp>
#include "sky_log.h"
#include "sky_utils.h"
extern struct service_info *s_info;
void _send_message_mq(std::string msg);

void skywalking_logging_report(std::string trace_id, std::string message, std::string level) {
    LogData log_data;
    auto time_now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    log_data.addTraceContext(trace_id);
    log_data.setTimestamp(time_now.count());
    log_data.addTag("level", level);
    log_data.addBody(TEXT, message);
    log_data.setService(s_info->service);
    log_data.setServiceInstance(s_info->service_instance);
    std::string msg = log_data.marshal();
    _send_message_mq(msg);
}

void skywalking_logging_report(LogData * log_data) {
    std::string msg = log_data->marshal();
    _send_message_mq(msg);
}

void _send_message_mq(std::string msg){
    if (strlen(s_info->service_instance) == 0) {
        sky_log("logging send message_queue fail:  opa server connection failure");
        return;
    }

    try { 
        boost::interprocess::message_queue mq(
            boost::interprocess::open_only,
            s_info->log_mq_name
        );

        int msg_length = static_cast<int>(msg.size());
        int max_length = SKYWALKING_G(logging_mq_max_message_length);
        if (msg_length > max_length) {
            sky_log("message is too big: " + std::to_string(msg_length) + ", mq_max_message_length=" + std::to_string(max_length));
            return;
        }      
        if (!mq.try_send(msg.data(), msg.size(), 0)) {
            sky_log("logging message_queue is fulled");
        }
    } catch (boost::interprocess::interprocess_exception &ex) {
        sky_log("logging message_queue ex" + std::string(ex.what()));
        php_error(E_WARNING, "%s %s", "[skywalking] logging open queue fail ", ex.what());
    }
}