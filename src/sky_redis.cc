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

#include "sky_redis.h"

std::map<std::string, redis_cmd_cb> commands = {
        {"DECR",   sky_redis_key_cmd},
        {"GET",    sky_redis_key_cmd},
        {"INCR",   sky_redis_key_cmd},
        {"STRLEN", sky_redis_key_cmd},
};

Span *sky_redis(zend_execute_data *execute_data, const std::string &class_name, const std::string &function_name) {

    auto *segment = static_cast<Segment *>(SKYWALKING_G(segment));
    auto *span = segment->createSpan(SkySpanType::Exit, SkySpanLayer::Cache, 7);
    span->setOperationName(class_name + "->" + function_name);
    span->addTag("db.type", "redis");

    // peer
    auto peer = sky_redis_peer(execute_data);
    if (!peer.empty()) {
        span->setPeer(peer);
    }

    if (commands.count(function_name) > 0) {
        std::string cmd;
        std::transform(function_name.begin(), function_name.end(), cmd.begin(), ::toupper);
        span->addTag("redis.command", commands[cmd](execute_data, cmd));
    }

    return span;
}

std::string sky_redis_peer(zend_execute_data *execute_data) {
    zval *command = &(execute_data->This);
    zval host;
    zval port;
#if PHP_VERSION_ID < 80000
    zend_call_method(command, Z_OBJCE_P(command), nullptr, ZEND_STRL("gethost"), &host, 0, nullptr, nullptr);
    zend_call_method(command, Z_OBJCE_P(command), nullptr, ZEND_STRL("getport"), &port, 0, nullptr, nullptr);
#else
    zend_call_method(Z_OBJCE_P(command), Z_OBJCE_P(command), nullptr, ZEND_STRL("gethost"), &host, 0, nullptr, nullptr);
    zend_call_method(Z_OBJCE_P(command), Z_OBJCE_P(command), nullptr, ZEND_STRL("getport"), &port, 0, nullptr, nullptr);
#endif

    if (!Z_ISUNDEF(host) && !Z_ISUNDEF(port) && Z_TYPE(host) == IS_STRING && Z_TYPE(port) == IS_LONG) {
        const char *char_host = ZSTR_VAL(Z_STR(host));
        return std::string(char_host) + ":" + std::to_string(Z_LVAL(port));
    }

    return nullptr;
}

std::string sky_redis_key_cmd(zend_execute_data *execute_data, std::string cmd) {
    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
    if (arg_count == 1) {
        zval *key = ZEND_CALL_ARG(execute_data, 1);
        if (Z_TYPE_P(key) == IS_STRING) {
            return cmd + " " + std::string(Z_STRVAL_P(key));
        }
    }

    return nullptr;
}
