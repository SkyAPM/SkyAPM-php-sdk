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



#include "sky_plugin_grpc.h"
#include "sky_core_segment.h"

#include "php_skywalking.h"
#include "sky_utils.h"

extern void (*ori_execute_ex)(zend_execute_data *execute_data);

SkyCoreSpan *sky_plugin_grpc(zend_execute_data *execute_data, char *class_name, char *function_name) {
    std::string _class_name(class_name);
    std::string _function_name(function_name);

    if (_function_name == "_simpleRequest" || _function_name == "_clientStreamRequest" ||
        _function_name == "_serverStreamRequest" || _function_name == "_bidiRequest") {

        auto *segment = sky_get_segment(execute_data, -1);
        if (segment->skip()) {
            return nullptr;
        }

        auto *span = segment->createSpan(SkyCoreSpanType::Exit, SkyCoreSpanLayer::RPCFramework, 23);
        span->setOperationName(_class_name + "->" + _function_name);
        span->addTag("rpc.type", "grpc");

        zval *method = ZEND_CALL_ARG(execute_data, 1);
        if (Z_TYPE_P(method) == IS_STRING) {
            span->addTag("rpc.method", Z_STRVAL_P(method));
        }

        zval *hostname = sky_read_property(&(execute_data->This), "hostname", 1);
        zval *hostname_override = sky_read_property(&(execute_data->This), "hostname_override", 1);

        if (hostname_override != nullptr && Z_TYPE_P(hostname_override) == IS_STRING) {
            span->setPeer(Z_STRVAL_P(hostname_override));
        } else if (hostname != nullptr && Z_TYPE_P(hostname) == IS_STRING) {
            span->setPeer(Z_STRVAL_P(hostname));
        }

        zval *metadata;
        int offset = 4;
        if (_function_name == "_simpleRequest" || _function_name == "_serverStreamRequest") {
            metadata = ZEND_CALL_ARG(execute_data, 4);
            offset = 4;
        } else if (_function_name == "_clientStreamRequest" || _function_name == "_bidiRequest") {
            metadata = ZEND_CALL_ARG(execute_data, 3);
            offset = 3;
        }

        std::string sw_header = segment->createHeader(span);
        if (nullptr != metadata) {
            if (Z_TYPE_P(metadata) == IS_UNDEF) {
                array_init(metadata);
                add_assoc_str(metadata, "sw8", zend_string_init(sw_header.c_str(), sw_header.length(), 0));
                ZVAL_ARR(ZEND_CALL_ARG(execute_data, offset), Z_ARR_P(metadata));
                execute_data->This.u2.num_args = offset;
            } else if (Z_TYPE_P(metadata) == IS_ARRAY) {
                SEPARATE_ARRAY(metadata);
                add_assoc_str(metadata, "sw8", zend_string_init(sw_header.c_str(), sw_header.length(), 0));
            }
        }

        ori_execute_ex(execute_data);
        span->setEndTIme();
        return span;
    }


    return nullptr;
}