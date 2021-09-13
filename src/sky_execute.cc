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


#include <iostream>
#include <thread>
#include <sstream>
#include "sky_execute.h"

#include "php_skywalking.h"

#include "sky_plugin_predis.h"
#include "sky_plugin_grpc.h"
#include "sky_plugin_redis.h"
#include "sky_plugin_memcached.h"
#include "sky_plugin_yar.h"
#include "sky_plugin_rabbit_mq.h"
#include "sky_plugin_hyperf_guzzle.h"
#include "sky_plugin_swoole_curl.h"
#include "sky_pdo.h"
#include "sky_plugin_mysqli.h"
#include "sky_module.h"
#include "segment.h"
#include "zend_API.h"

void (*ori_execute_ex)(zend_execute_data *execute_data) = nullptr;

void (*ori_execute_internal)(zend_execute_data *execute_data, zval *return_value) = nullptr;

void sky_execute_ex(zend_execute_data *execute_data) {

    zend_function *fn = execute_data->func;
    int is_class = fn->common.scope != nullptr && fn->common.scope->name != nullptr;
    char *class_name = is_class ? ZSTR_VAL(fn->common.scope->name) : nullptr;
    char *function_name = fn->common.function_name != nullptr ? ZSTR_VAL(fn->common.function_name) : nullptr;
    std::string className = "";
    std::string methodName = "";
    if (class_name != nullptr) className = class_name;
    if (function_name != nullptr) methodName = function_name;

    // swoole
    bool swoole = false;
    int64_t request_id = -1;
    bool afterExec = true;
    zval *sw_response;
    if (SKY_IS_SWOOLE(function_name) || SKY_IS_SWOOLE_FRAMEWORK(class_name, function_name)) {
        uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
        if (arg_count == 2) {
            zval *sw_request = ZEND_CALL_ARG(execute_data, 1);
            sw_response = ZEND_CALL_ARG(execute_data, 2);
            if (Z_TYPE_P(sw_request) == IS_OBJECT && Z_TYPE_P(sw_response) == IS_OBJECT) {
                if (strcmp(ZSTR_VAL(Z_OBJ_P(sw_request)->ce->name), "Swoole\\Http\\Request") == 0) {
                    if (strcmp(ZSTR_VAL(Z_OBJ_P(sw_response)->ce->name), "Swoole\\Http\\Response") == 0) {
                        swoole = true;
                        zval *z_fd = sky_read_property(sw_request, "fd", 0);
                        SKYWALKING_G(is_swoole) = true;
                        request_id = Z_LVAL_P(z_fd);
                        sky_request_init(sw_request, request_id);
                    }
                }
            }
        }

        // swoft的rpc请求
        if (arg_count == 4 && SKY_IS_SWOFT_RPC(class_name, function_name)){
            zval *server = ZEND_CALL_ARG(execute_data, 1);
            zval *fd = ZEND_CALL_ARG(execute_data, 2);
            zval *reactorId = ZEND_CALL_ARG(execute_data, 3);
            zval *data = ZEND_CALL_ARG(execute_data, 4);
            std::string swfHost;
            std::string swfPort = "";
            if (SKY_IS_OBJECT(server)){

                zval *host = sky_read_property(server, "host", 0);
                zval *port = sky_read_property(server, "port", 0);
                swfHost = Z_STRVAL_P(host);
                if (Z_TYPE_P(port) == IS_STRING) swfPort = Z_STRVAL_P(port);
                if (Z_TYPE_P(port) == IS_LONG) swfPort = std::to_string(Z_LVAL_P(port));
            }

            request_id = Z_LVAL_P(fd);
            std::string header = "";
            std::string traceData = ZSTR_VAL(Z_STR_P(data));

            // decode json
            zval jsonData;
            sky_json_decode(traceData, &jsonData);
            zval ext;
            zval *extData;
            ZVAL_NEW_ARR(&ext);
            extData = sky_hashtable_default(&jsonData, "ext", &ext);

            swoft_json_rpc jsonRpcData;
//            php_var_dump(sky_hashtable_default(extData, "sw8", "")); // s8的支持，二期标准化sw8协议,molten端已经实现，这次先这样@210821
            jsonRpcData.method = Z_STRVAL_P(sky_hashtable_default(&jsonData, "method", ""));//jsonValue["method"].asString();
            jsonRpcData.jsonrpc = Z_STRVAL_P(sky_hashtable_default(&jsonData, "jsonrpc", ""));//jsonValue["jsonrpc"].asString();
            jsonRpcData.ext.traceid = Z_STRVAL_P(sky_hashtable_default(extData, "traceid", ""));//extData["traceid"].asString();
            jsonRpcData.ext.spanid = Z_STRVAL_P(sky_hashtable_default(extData, "spanid", ""));//extData["spanid"].asString();
            jsonRpcData.ext.parentid = Z_STRVAL_P(sky_hashtable_default(extData, "parentid", "0"));//extData["parentid"].asString();
            jsonRpcData.ext.uri = Z_STRVAL_P(sky_hashtable_default(extData, "uri", ""));//extData["uri"].asString();
            jsonRpcData.ext.requestTime = Z_STRVAL_P(sky_hashtable_default(extData, "requestTime", ""));//extData["requestTime"].asString();
            jsonRpcData.ext.serviceName = Z_STRVAL_P(sky_hashtable_default(extData, "serviceName", ""));//extData["serviceName"].asString();
            jsonRpcData.ext.ServiceInstance = Z_STRVAL_P(sky_hashtable_default(extData, "ServiceInstance", "0"));//extData["ServiceInstance"].asString();
            jsonRpcData.ext.address = swfHost + ":" + swfPort;
            jsonRpcData.ext.endpoint = swfHost + ":" + swfPort;

            // segments全局变量
            if (!jsonRpcData.ext.traceid.empty()){
                sky_rpc_init(request_id, jsonRpcData);
                swoole = true;
                SKYWALKING_G(is_swoole) = true;
            }
        }
    }

    if (SKYWALKING_G(is_swoole)) {

        if (sky_get_segment(execute_data, request_id) == nullptr) {
            ori_execute_ex(execute_data);
            return;
        }
    } else {
        if (sky_get_segment(execute_data, 0) == nullptr) {
            ori_execute_ex(execute_data);
            return;
        }
    }

    Span *span = nullptr;

    if (class_name != nullptr && function_name != nullptr) {
        if (strcmp(function_name, "executeCommand") == 0) {
            span = sky_predis(execute_data, class_name, function_name);
        } else if (strcmp(class_name, "Grpc\\BaseStub") == 0) {
            afterExec = false;
            span = sky_plugin_grpc(execute_data, class_name, function_name);
        } else if (strcmp(class_name, "PhpAmqpLib\\Channel\\AMQPChannel") == 0) {
            span = sky_plugin_rabbit_mq(execute_data, class_name, function_name);
        } else if ((SKY_STRCMP(class_name, "Hyperf\\Guzzle\\CoroutineHandler") ||
                    SKY_STRCMP(class_name, "Hyperf\\Guzzle\\PoolHandler")) &&
                   SKY_STRCMP(function_name, "__invoke")) {
            afterExec = false;
            span = sky_plugin_hyperf_guzzle(execute_data, class_name, function_name);
        }
    } else if (function_name != nullptr) {
        if (SKY_STRCMP(function_name, "swoole_curl_exec")) {
            afterExec = false;
            sky_plugin_swoole_curl(execute_data, "", function_name);
        }
    }

    if (afterExec) {
        ori_execute_ex(execute_data);
        if (span != nullptr) {
            span->setEndTIme();
        }
    }
    if (swoole) {
        sky_request_flush(sw_response, request_id);
    }
}

void sky_execute_internal(zend_execute_data *execute_data, zval *return_value) {

    if (SKYWALKING_G(is_swoole)) {
        if (sky_get_segment(execute_data, -1) == nullptr) {
            if (ori_execute_internal) {
                ori_execute_internal(execute_data, return_value);
            } else {
                execute_internal(execute_data, return_value);
            }
            return;
        }
    } else {
        if (sky_get_segment(execute_data, 0) == nullptr) {
            if (ori_execute_internal) {
                ori_execute_internal(execute_data, return_value);
            } else {
                execute_internal(execute_data, return_value);
            }
            return;
        }
    }

    zend_function *fn = execute_data->func;
    int is_class = fn->common.scope != nullptr && fn->common.scope->name != nullptr;
    char *class_name = is_class ? ZSTR_VAL(fn->common.scope->name) : nullptr;
    char *function_name = fn->common.function_name != nullptr ? ZSTR_VAL(fn->common.function_name) : nullptr;

    if (class_name != nullptr && function_name != nullptr) {
        if (strcmp(class_name, "Swoole\\Http\\Response") == 0 && strcmp(function_name, "status") == 0) {
            auto *segment = sky_get_segment(execute_data, -1);;
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
        } else if (strcmp(class_name, "mysqli") == 0){
            span = sky_plugin_mysqli(execute_data, class_name, function_name);
        } else if (strcmp(class_name, "Redis") == 0) {
            span = sky_plugin_redis(execute_data, class_name, function_name);
        } else if (strcmp(class_name, "Memcached") == 0) {
          span = sky_plugin_memcached(execute_data, class_name, function_name);
        } else if (strcmp(class_name, "Yar_Client") == 0) {
          span = sky_plugin_yar_client(execute_data, class_name, function_name);
        } else if (strcmp(class_name, "Yar_Server") == 0) {
          span = sky_plugin_yar_server(execute_data, class_name, function_name);
        }
    } else if (function_name != nullptr) {
        if (strcmp(function_name, "mysqli_") > 0) {
            span = sky_plugin_mysqli(execute_data, "", function_name);
        }
    }

    if (ori_execute_internal) {
        ori_execute_internal(execute_data, return_value);
    } else {
        execute_internal(execute_data, return_value);
    }

    if (span != nullptr) {
        // catch errors add to span log
        if (class_name != nullptr) {
            if (strcmp(class_name, "PDO") == 0 || strcmp(class_name, "PDOStatement") == 0) {
                if (Z_TYPE_P(return_value) == IS_FALSE) {
                    span->setIsError(true);
                    sky_pdo_check_errors(execute_data, span);
                }
            } else if (strcmp(class_name, "mysqli") == 0){
                if (Z_TYPE_P(return_value) == IS_FALSE) { 
                    span->setIsError(true);  
                    sky_plugin_mysqli_check_errors(execute_data, span, 1);
                }
            }
        } else if (function_name != nullptr) {
            if (strcmp(function_name, "mysqli_") > 0) {
                if (Z_TYPE_P(return_value) == IS_FALSE) { 
                    span->setIsError(true);  
                    sky_plugin_mysqli_check_errors(execute_data, span, 0);
                }
            }

        }
        span->setEndTIme();
    }
}
