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


#include "sky_plugin_predis.h"

#include "php_skywalking.h"

#include "sky_core_segment.h"
#include "sky_utils.h"

SkyCoreSpan *sky_predis(zend_execute_data *execute_data, const std::string &class_name, const std::string &function_name) {
    if (class_name != "Predis\\Connection\\AbstractConnection") {
        return nullptr;
    }

    uint32_t args = ZEND_CALL_NUM_ARGS(execute_data);
    if (args) {
        auto *segment = sky_get_segment(execute_data, -1);
        if (segment->skip()) {
            return nullptr;
        }
        auto *span = segment->createSpan(SkyCoreSpanType::Exit, SkyCoreSpanLayer::Cache, 8006);

        zval *command = ZEND_CALL_ARG(execute_data, 1);

        zval *id = (zval *) emalloc(sizeof(zval));
        zval *arguments = (zval *) emalloc(sizeof(zval));
#if PHP_VERSION_ID < 80000
        zend_call_method(command, Z_OBJCE_P(command), nullptr, ZEND_STRL("getid"), id, 0, nullptr, nullptr);
        zend_call_method(command, Z_OBJCE_P(command), nullptr, ZEND_STRL("getarguments"), arguments, 0, nullptr,
                         nullptr);
#else
        zend_call_method(Z_OBJ_P(command), Z_OBJCE_P(command), nullptr, ZEND_STRL("getid"), id, 0, nullptr,
                         nullptr);
        zend_call_method(Z_OBJ_P(command), Z_OBJCE_P(command), nullptr, ZEND_STRL("getarguments"), arguments,
                         0, nullptr, nullptr);
#endif

        if (id != nullptr && Z_TYPE_P(id) == IS_STRING) {
            span->setOperationName(class_name + "->" + std::string(Z_STRVAL_P(id)));
            span->addTag("db.type", "redis");
            // other
            auto peer = sky_predis_peer(execute_data);
            if (!peer.empty()) {
                span->setPeer(peer);
            }

            // cmd
            auto cmd = sky_predis_command(id, arguments);
            if (!cmd.empty()) {
                span->addTag("redis.command", cmd);
            }
        }

        if (id) {
            zval_ptr_dtor(id);
            efree(id);
        }

        if (arguments) {
            zval_ptr_dtor(arguments);
            efree(arguments);
        }

        return span;
    }
    return nullptr;
}

std::string sky_predis_peer(zend_execute_data *execute_data) {

    zval *parameters_class = sky_read_property(&(execute_data->This), "parameters", 0);
    if (SKY_PREDIS_IS_PARAMETERS(parameters_class)) {
        zval *parameters = sky_read_property(parameters_class, "parameters", 0);
        zval *predis_host = zend_hash_str_find(Z_ARRVAL_P(parameters), "host", sizeof("host") - 1);
        zval *predis_port = zend_hash_str_find(Z_ARRVAL_P(parameters), "port", sizeof("port") - 1);
        zval port;
        ZVAL_COPY(&port, predis_port);
        if (Z_TYPE(port) != IS_LONG) {
            convert_to_long(&port);
        }

        if (Z_TYPE_P(predis_host) == IS_STRING && Z_TYPE(port) == IS_LONG) {
            char *host = ZSTR_VAL(Z_STR_P(predis_host));
            return std::string(host) + ":" + std::to_string(Z_LVAL(port));
        }
    }
    return "";
}

std::string sky_predis_command(zval *id, zval *arguments) {
    if (arguments != nullptr && Z_TYPE_P(arguments) == IS_ARRAY) {
        zend_ulong key;
        zval *value, dup_value;
        std::string command;
        command += std::string(Z_STRVAL_P(id)) + " ";
        ZEND_HASH_FOREACH_NUM_KEY_VAL(Z_ARRVAL_P(arguments), key, value)
                {
                    switch (Z_TYPE_P(value)) {
                        case IS_STRING:
                            command += std::string(Z_STRVAL_P(value)) + " ";
                            break;
                        case IS_ARRAY:
                            break;
                        default:
                            ZVAL_COPY(&dup_value, value);
                            convert_to_string(&dup_value);
                            command += std::string(Z_STRVAL_P(&dup_value)) + " ";
                            break;
                    }
                }
        ZEND_HASH_FOREACH_END();

        return command;
    }

    return "";
}
