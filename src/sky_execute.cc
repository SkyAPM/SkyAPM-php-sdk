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

#include <iostream>
#include "sky_execute.h"

#include "php_skywalking.h"

#include "sky_predis.h"
#include "sky_grpc.h"
#include "sky_plugin_rabbit_mq.h"
#include "sky_pdo.h"
#include "sky_module.h"
#include "segment.h"

void (*ori_execute_ex)(zend_execute_data *execute_data) = nullptr;

void (*ori_execute_internal)(zend_execute_data *execute_data, zval *return_value) = nullptr;

void sky_execute_ex(zend_execute_data *execute_data) {

    zend_function *fn = execute_data->func;
    int is_class = fn->common.scope != nullptr && fn->common.scope->name != nullptr;
    char *class_name = is_class ? ZSTR_VAL(fn->common.scope->name) : nullptr;
    char *function_name = fn->common.function_name != nullptr ? ZSTR_VAL(fn->common.function_name) : nullptr;

    // swoole
    bool swoole = false;
    zval *sw_response;
    if (function_name != nullptr && strcmp(function_name, "{closure}") == 0) {
        uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
        if (arg_count == 2) {
            zval *sw_request = ZEND_CALL_ARG(execute_data, 1);
            sw_response = ZEND_CALL_ARG(execute_data, 2);
            if (Z_TYPE_P(sw_request) == IS_OBJECT && Z_TYPE_P(sw_response) == IS_OBJECT) {
                if (strcmp(ZSTR_VAL(Z_OBJ_P(sw_request)->ce->name), "Swoole\\Http\\Request") == 0) {
                    if(strcmp(ZSTR_VAL(Z_OBJ_P(sw_response)->ce->name), "Swoole\\Http\\Response") == 0) {
                        swoole = true;
                        sky_request_init(sw_request);
                    }
                }
            }
        }
    }

    if (SKYWALKING_G(segment) == nullptr) {
        ori_execute_ex(execute_data);
        return;
    }

    Span *span = nullptr;

    if (class_name != nullptr) {
        if (strcmp(class_name, "Predis\\Client") == 0 && strcmp(function_name, "executeCommand") == 0) {
            span = sky_predis(execute_data, class_name, function_name);
        } else if (strcmp(class_name, "Grpc\\BaseStub") == 0) {
            span = sky_grpc(execute_data, class_name, function_name);
        } else if (strcmp(class_name, "PhpAmqpLib\\Channel\\AMQPChannel") == 0) {
            span = sky_plugin_rabbit_mq(execute_data, class_name, function_name);
        }
    }

    if (span != nullptr) {
        ori_execute_ex(execute_data);
        span->setEndTIme();
    } else {
        ori_execute_ex(execute_data);
    }

    if (swoole) {
        sky_request_flush(sw_response);
        zval_dtor(&SKYWALKING_G(curl_header));
    }
}

void sky_execute_internal(zend_execute_data *execute_data, zval *return_value) {
    if (SKYWALKING_G(segment) == nullptr) {
        if (ori_execute_internal) {
            ori_execute_internal(execute_data, return_value);
        } else {
            execute_internal(execute_data, return_value);
        }
    } else {
        zend_function *fn = execute_data->func;
        int is_class = fn->common.scope != nullptr && fn->common.scope->name != nullptr;
        char *class_name = is_class ? ZSTR_VAL(fn->common.scope->name) : nullptr;
        char *function_name = fn->common.function_name != nullptr ? ZSTR_VAL(fn->common.function_name) : nullptr;

        if (class_name != nullptr && function_name != nullptr) {
            if (strcmp(class_name, "Swoole\\Http\\Response") == 0 && strcmp(function_name, "status") == 0) {
                auto *segment = static_cast<Segment *>(SKYWALKING_G(segment));
                uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
                if (arg_count >= 1) {
                    zval *status = ZEND_CALL_ARG(execute_data, 1);
                    if (Z_TYPE_P(status) == IS_LONG) {
                        segment->setStatusCode(Z_LVAL_P(status));
                    }
                }
            }
        }

        Span *span = nullptr;
        if (class_name != nullptr) {
            if (strcmp(class_name, "PDO") == 0 || strcmp(class_name, "PDOStatement") == 0) {
                span = sky_pdo(execute_data, class_name, function_name);
            }
        }

        if (ori_execute_internal) {
            ori_execute_internal(execute_data, return_value);
        } else {
            execute_internal(execute_data, return_value);
        }

        if (span != nullptr) {
            span->setEndTIme();
        }
    }
}