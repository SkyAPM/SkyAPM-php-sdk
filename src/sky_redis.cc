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

Span *sky_redis(zend_execute_data *execute_data, char *class_name, char *function_name) {
    std::string _class_name(class_name);
    std::string _function_name(function_name);

    uint32_t args = ZEND_CALL_NUM_ARGS(execute_data);
    char *fnamewall = sky_redis_fnamewall(_function_name.c_str());
    if (args && sky_redis_opt_for_string_key(fnamewall) == 1) {
        auto *segment = static_cast<Segment *>(SKYWALKING_G(segment));
        auto *span = segment->createSpan(SkySpanType::Exit, SkySpanLayer::Cache, 7);
        span->setOperationName(_class_name + "->" + _function_name);
        span->addTag("db.type", "redis");

        // peer
        auto peer = sky_redis_peer(execute_data);
        if (!peer.empty()) {
            span->setPeer(peer);
        }

        // key and command
        sky_redis_command(span, execute_data, function_name);

        efree(fnamewall);
        return span;
    }   

    efree(fnamewall);
    return nullptr;
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
        return std::string(char_host)+":"+std::to_string(Z_LVAL(port));
    }

    return nullptr;
}

void sky_redis_command(Span *span, zend_execute_data *execute_data, char *function_name){
    smart_str command = {0};
    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
    char *fname = zend_str_tolower_dup(function_name, strlen(function_name));
    smart_str_appends(&command, fname);
    smart_str_appends(&command, " ");
    efree(fname);

    int is_string_command = 1;
    uint32_t i;
    for (i = 1; i < arg_count + 1; ++i)
    {
        zval str_p;
        zval *p = ZEND_CALL_ARG(execute_data, i);
        if (Z_TYPE_P(p) == IS_ARRAY){
            is_string_command = 0;
            break;
        }

        ZVAL_COPY(&str_p, p);
        if (Z_TYPE_P(&str_p) != IS_STRING){
            convert_to_string(&str_p);
        }

        if (i == 1){
            span->addTag("redis.key", Z_STRVAL_P(&str_p));
        }

        char *tmp = zend_str_tolower_dup(Z_STRVAL_P(&str_p), Z_STRLEN_P(&str_p));
        smart_str_appends(&command, tmp);
        smart_str_appends(&command, " ");
        efree(tmp);
    }

    // store command to tags
    if (command.s) {
        smart_str_0(&command);
        if (is_string_command){
            std::string str_cmd = std::string(ZSTR_VAL(command.s));
            int pos = str_cmd.find(" ");
            if (pos >= 0) {
                span->addTag("redis.command", str_cmd.substr(0, pos));
            }
        }
        smart_str_free(&command);
    }
}

int sky_redis_opt_for_string_key(char *fnamewall) {
    if (strstr(REDIS_KEY_STRING, fnamewall) || \
        strstr(REDIS_KEY_KEY, fnamewall) || \
        strstr(REDIS_KEY_HASH, fnamewall) || strstr(REDIS_KEY_LIST, fnamewall) || \
        strstr(REDIS_KEY_SET, fnamewall) || strstr(REDIS_KEY_SORT, fnamewall) || \
        strstr(REDIS_KEY_HLL, fnamewall) || strstr(REDIS_KEY_GEO, fnamewall))
    {
        return 1;
    }
    return 0;
}

char *sky_redis_fnamewall(const char *function_name) {
    char *fnamewall = (char *)emalloc(strlen(function_name) + 3);
    bzero(fnamewall, strlen(function_name) + 3);
    sprintf(fnamewall, "|%s|", function_name);
    char *fnamewall_lower = zend_str_tolower_dup(fnamewall, strlen(fnamewall));
    efree(fnamewall);
    return fnamewall_lower;
}
