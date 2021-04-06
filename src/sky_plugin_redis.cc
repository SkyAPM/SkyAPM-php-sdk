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

#include "sky_plugin_redis.h"

std::map<std::string, redis_cmd_cb> commands = {
        {"APPEND",      sky_plugin_redis_key_value_cmd},

        {"BITCOUNT",    sky_plugin_redis_bit_count_cmd},
        {"BITFIELD",    sky_plugin_redis_todo_cmd},
        {"BITOP",       sky_plugin_redis_todo_cmd},
        {"BITPOS",      sky_plugin_redis_todo_cmd},

        {"DECR",        sky_plugin_redis_key_cmd},
        {"DECRBY",      sky_plugin_redis_todo_cmd},

        {"GET",         sky_plugin_redis_key_cmd},
        {"GETBIT",      sky_plugin_redis_todo_cmd},
        {"GETRANGE",    sky_plugin_redis_todo_cmd},
        {"GETSET",      sky_plugin_redis_key_value_cmd},

        {"INCR",        sky_plugin_redis_key_cmd},
        {"INCRBY",      sky_plugin_redis_todo_cmd},
        {"INCRBYFLOAT", sky_plugin_redis_todo_cmd},

        {"MGET",        sky_plugin_redis_multi_key_cmd},
        {"MSET",        sky_plugin_redis_todo_cmd},
        {"MSETNX",      sky_plugin_redis_todo_cmd},
        {"PSETEX",      sky_plugin_redis_todo_cmd},
        {"GETMULTIPLE", sky_plugin_redis_multi_key_cmd},

        {"SET",         sky_plugin_redis_set_cmd},
        {"SETBIT",      sky_plugin_redis_todo_cmd},
        {"SETEX",       sky_plugin_redis_setex_cmd},
        {"SETNX",       sky_plugin_redis_key_value_cmd},
        {"SETRANGE",    sky_plugin_redis_todo_cmd},

        {"STRALGO",     sky_plugin_redis_todo_cmd},
        {"STRLEN",      sky_plugin_redis_key_cmd},

        {"EVAL",        sky_plugin_redis_key_cmd},

        {"DEL",         sky_plugin_redis_uncertain_keys_cmd},
        {"DELETE",      sky_plugin_redis_uncertain_keys_cmd},
        {"UNLINK",      sky_plugin_redis_uncertain_keys_cmd},
        {"EXISTS",      sky_plugin_redis_uncertain_keys_cmd},

        // empty commands
        {"PING",        sky_plugin_redis_todo_cmd},
        {"PIPELINE",    sky_plugin_redis_todo_cmd},
        {"MULTI",       sky_plugin_redis_todo_cmd},
        {"DISCARD",     sky_plugin_redis_todo_cmd},
        {"EXEC",        sky_plugin_redis_todo_cmd},

        {"EXPIRE",      sky_plugin_redis_key_ttl_cmd},
        {"PEXPIRE",     sky_plugin_redis_key_ttl_cmd},
        {"EXPIREAT",    sky_plugin_redis_key_ttl_cmd},
        {"PEXPIREAT",   sky_plugin_redis_key_ttl_cmd},
        {"SETTIMEOUT",  sky_plugin_redis_key_ttl_cmd},

};

Span *sky_plugin_redis(zend_execute_data *execute_data, const std::string &class_name, const std::string &function_name) {

    std::string cmd = function_name;
    std::transform(function_name.begin(), function_name.end(), cmd.begin(), ::toupper);
    if (commands.count(cmd) > 0) {
        auto *segment = sky_get_segment(execute_data, -1);
        if (segment) {
            auto *span = segment->createSpan(SkySpanType::Exit, SkySpanLayer::Cache, 7);
            span->setOperationName(class_name + "->" + function_name);
            span->addTag("db.type", "redis");

            // peer
            auto peer = sky_plugin_redis_peer(execute_data);
            if (!peer.empty()) {
                span->setPeer(peer);
            }

            span->addTag("redis.command", commands[cmd](execute_data, cmd));
            return span;
        }
    }

    return nullptr;
}

std::string sky_plugin_redis_peer(zend_execute_data *execute_data) {
    zval *command = &(execute_data->This);
    zval host;
    zval port;
#if PHP_VERSION_ID < 80000
    zend_call_method(command, Z_OBJCE_P(command), nullptr, ZEND_STRL("gethost"), &host, 0, nullptr, nullptr);
    zend_call_method(command, Z_OBJCE_P(command), nullptr, ZEND_STRL("getport"), &port, 0, nullptr, nullptr);
#else
    zend_call_method(Z_OBJ_P(command), Z_OBJCE_P(command), nullptr, ZEND_STRL("gethost"), &host, 0, nullptr, nullptr);
    zend_call_method(Z_OBJ_P(command), Z_OBJCE_P(command), nullptr, ZEND_STRL("getport"), &port, 0, nullptr, nullptr);
#endif

    if (!Z_ISUNDEF(host) && !Z_ISUNDEF(port) && Z_TYPE(host) == IS_STRING) {
        std::string peer(Z_STRVAL(host));

        if (Z_TYPE(port) == IS_LONG) {
            peer += ":" + std::to_string(Z_LVAL(port));
        } else if (Z_TYPE(port) == IS_STRING) {
            peer += ":" + std::string(Z_STRVAL(port));
        } else {
            peer += ":6379";
        }

        zval_dtor(&host);
        zval_dtor(&port);

        return peer;
    }

    return "";
}

std::string sky_plugin_redis_key_cmd(zend_execute_data *execute_data, std::string cmd) {
    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
    if (arg_count == 1) {
        zval *key = ZEND_CALL_ARG(execute_data, 1);
        if (Z_TYPE_P(key) == IS_STRING) {
            return cmd + " " + std::string(Z_STRVAL_P(key));
        }
    }

    return "";
}

std::string sky_plugin_redis_key_value_cmd(zend_execute_data *execute_data, std::string cmd) {
    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
    if (arg_count == 2) {
        zval *key = ZEND_CALL_ARG(execute_data, 1);
        zval *value = ZEND_CALL_ARG(execute_data, 2);
        if (Z_TYPE_P(key) == IS_STRING && Z_TYPE_P(value) == IS_STRING) {
            return cmd + " " + std::string(Z_STRVAL_P(key)) + " " + std::string(Z_STRVAL_P(value));
        }
    }
    return "";
}

std::string sky_plugin_redis_set_cmd(zend_execute_data *execute_data, std::string cmd) {
    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
    if (arg_count == 2) {
        return sky_plugin_redis_key_value_cmd(execute_data, cmd);
    }
    if (arg_count == 3) {
        zval *key = ZEND_CALL_ARG(execute_data, 1);
        zval *value = ZEND_CALL_ARG(execute_data, 2);
        zval *optional = ZEND_CALL_ARG(execute_data, 3);

        if (Z_TYPE_P(key) == IS_STRING && Z_TYPE_P(value) == IS_STRING) {
            std::string _cmd = cmd + " " + std::string(Z_STRVAL_P(key)) + " " + std::string(Z_STRVAL_P(value));
            
            if (Z_TYPE_P(optional) == IS_LONG) {
                return _cmd + " " + std::to_string(Z_LVAL_P(optional));
            }

            if (Z_TYPE_P(optional) == IS_ARRAY) {
                zend_ulong nk;
                zend_string *sk;
                zval *val;
                ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(optional), nk, sk, val) {
                    if (sk == NULL && Z_TYPE_P(val) == IS_STRING) {
                        _cmd += " " + std::string(Z_STRVAL_P(val));
                    } else if (Z_TYPE_P(val) == IS_LONG) {
                        _cmd += " " + std::string(ZSTR_VAL(sk)) + " " + std::to_string(Z_LVAL_P(val));
                    }
                } ZEND_HASH_FOREACH_END();
                return _cmd;
            }
        }
    }
    return "";
}

std::string sky_plugin_redis_setex_cmd(zend_execute_data *execute_data, std::string cmd) {
    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
    if (arg_count == 3) {
        zval *key = ZEND_CALL_ARG(execute_data, 1);
        zval *ttl = ZEND_CALL_ARG(execute_data, 2);
        zval *value = ZEND_CALL_ARG(execute_data, 3);

        if (Z_TYPE_P(key) == IS_STRING && Z_TYPE_P(ttl) == IS_LONG && Z_TYPE_P(value) == IS_STRING) {
            return cmd + " " + std::string(Z_STRVAL_P(key)) + " " + std::to_string(Z_LVAL_P(ttl)) + " " + std::string(Z_STRVAL_P(value));
        }
    }
    return "";
}

std::string sky_plugin_redis_multi_key_cmd(zend_execute_data *execute_data, std::string cmd) {
    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
    if (arg_count == 1) {
        zval *keys = ZEND_CALL_ARG(execute_data, 1);
        if (Z_TYPE_P(keys) == IS_ARRAY) {
            std::string _cmd = cmd;
            zend_ulong index;
            zval *key;
            ZEND_HASH_FOREACH_NUM_KEY_VAL(Z_ARRVAL_P(keys), index, key) {
                if (Z_TYPE_P(key) == IS_STRING) {
                    _cmd += " " + std::string(Z_STRVAL_P(key));
                }
            } ZEND_HASH_FOREACH_END();
            return _cmd;
        }
    }
    return "";
}

std::string sky_plugin_redis_key_ttl_cmd(zend_execute_data *execute_data, std::string cmd) {
    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
    if (arg_count == 2) {
        zval *key = ZEND_CALL_ARG(execute_data, 1);
        zval *ttl = ZEND_CALL_ARG(execute_data, 2);
        if (Z_TYPE_P(key) == IS_STRING && Z_TYPE_P(ttl) == IS_LONG) {
            return cmd + " " + std::string(Z_STRVAL_P(key)) + " " + std::to_string(Z_LVAL_P(ttl));
        }
    }
    return "";
}

std::string sky_plugin_redis_uncertain_keys_cmd(zend_execute_data *execute_data, std::string cmd) {
    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);

    if (arg_count == 1) {
        zval *keys = ZEND_CALL_ARG(execute_data, 1);
        if (Z_TYPE_P(keys) == IS_STRING) {
            return cmd + " " + std::string(Z_STRVAL_P(keys));
        }
        if (Z_TYPE_P(keys) == IS_ARRAY) {
            std::string _cmd = cmd;
            zend_ulong index;
            zval *key;
            ZEND_HASH_FOREACH_NUM_KEY_VAL(Z_ARRVAL_P(keys), index, key) {
                if (Z_TYPE_P(key) == IS_STRING) {
                    _cmd += " " + std::string(Z_STRVAL_P(key));
                }
            } ZEND_HASH_FOREACH_END();
            return _cmd;
        }
    }

    if (arg_count > 1) {
        uint32_t i = 0;
        std::string _cmd = cmd;
        for (i = 1; i <= arg_count; i++) {
            zval *key = ZEND_CALL_ARG(execute_data, i);
            if (Z_TYPE_P(key) == IS_STRING) {
                _cmd += " " + std::string(Z_STRVAL_P(key));
            }
        }
        return _cmd;
    }

    return "";
}

std::string sky_plugin_redis_todo_cmd(zend_execute_data *execute_data, std::string cmd) {
    return "";
}

std::string sky_plugin_redis_bit_count_cmd(zend_execute_data *execute_data, std::string cmd) {
    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);

    if (arg_count >= 1) {
        for (uint32_t i = 1; i <= 3; ++i) {
            if (i <= arg_count) {
                zval *value = ZEND_CALL_ARG(execute_data, i);
                if (Z_TYPE_P(value) == IS_LONG) {
                    cmd += " " + std::to_string(Z_LVAL_P(value));

                    if (arg_count == 2) {
                        cmd += " -1";
                    }

                } else if (Z_TYPE_P(value) == IS_STRING) {
                    cmd += " " + std::string(Z_STRVAL_P(value));
                }
            }
        }
        return cmd;
    }
    return "";
}
