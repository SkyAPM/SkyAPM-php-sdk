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


#include "sky_plugin_rabbit_mq.h"
#include "sky_core_segment.h"
#include "sky_utils.h"

SkyCoreSpan *sky_plugin_rabbit_mq(zend_execute_data *execute_data, const std::string &class_name, const std::string &function_name) {

    if (function_name == "basic_publish") {
        auto *segment = sky_get_segment(execute_data, -1);
        if (segment->skip()) {
            return nullptr;
        }
        auto *span = segment->createSpan(SkyCoreSpanType::Exit, SkyCoreSpanLayer::MQ, 52);

        span->setOperationName(class_name + "->" + function_name);

        zval *exchange = ZEND_CALL_ARG(execute_data, 2);

        //span->addTag("mq.queue", "queueName");
        if (Z_TYPE_P(exchange) == IS_STRING) {
            span->addTag("mq.topic", Z_STRVAL_P(exchange));
        }

        // peer
        zval *connection = sky_read_property(&(execute_data->This), "connection", 1);
        if (Z_TYPE_P(connection) == IS_OBJECT) {
            zval *params = sky_read_property(connection, "construct_params", 1);
            if (Z_TYPE_P(params) == IS_ARRAY) {
                zval *host = zend_hash_index_find(Z_ARRVAL_P(params), 0);
                zval *port = zend_hash_index_find(Z_ARRVAL_P(params), 1);

                if (Z_TYPE_P(host) == IS_STRING) {
                    std::string peer = std::string(Z_STRVAL_P(host)) + ":";

                    if (Z_TYPE_P(port) == IS_STRING) {
                        peer += std::string(Z_STRVAL_P(port));
                    } else if (Z_TYPE_P(port) == IS_LONG) {
                        peer += std::to_string(Z_LVAL_P(port));
                    } else {
                        peer += "5672";
                    }

                    span->setPeer(peer);
                    span->addTag("mq.broker", peer);
                }
            }
        }
        return span;
    } else {
        return nullptr;
    }
}
