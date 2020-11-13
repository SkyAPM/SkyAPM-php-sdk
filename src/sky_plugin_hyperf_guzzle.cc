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

#include "sky_plugin_hyperf_guzzle.h"
#include "php_skywalking.h"
#include "segment.h"
#include "sky_utils.h"

extern void (*ori_execute_ex)(zend_execute_data *execute_data);

Span *sky_plugin_hyperf_guzzle(zend_execute_data *execute_data, const std::string &class_name, const std::string &function_name) {

    auto *segment = static_cast<Segment *>(SKYWALKING_G(segment));
    auto *span = segment->createSpan(SkySpanType::Exit, SkySpanLayer::Http, 8002);

    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);

    if (arg_count >= 1) {
        zval *request = ZEND_CALL_ARG(execute_data, 1);
        zval uri;
        zend_call_method(request, Z_OBJCE_P(request), nullptr, ZEND_STRL("geturi"), &uri, 0, nullptr, nullptr);

        if (!Z_ISUNDEF(uri)) {
            zval scheme, host, port, path, query, to_string;
            int _port = 80;
            zend_call_method(&uri, Z_OBJCE_P(&uri), nullptr, ZEND_STRL("getscheme"), &scheme, 0, nullptr, nullptr);
            zend_call_method(&uri, Z_OBJCE_P(&uri), nullptr, ZEND_STRL("gethost"), &host, 0, nullptr, nullptr);
            zend_call_method(&uri, Z_OBJCE_P(&uri), nullptr, ZEND_STRL("getport"), &port, 0, nullptr, nullptr);
            zend_call_method(&uri, Z_OBJCE_P(&uri), nullptr, ZEND_STRL("getpath"), &path, 0, nullptr, nullptr);
            zend_call_method(&uri, Z_OBJCE_P(&uri), nullptr, ZEND_STRL("getquery"), &query, 0, nullptr, nullptr);
            zend_call_method(&uri, Z_OBJCE_P(&uri), nullptr, ZEND_STRL("__tostring"), &to_string, 0, nullptr, nullptr);

            if (!Z_ISUNDEF(scheme) && Z_TYPE(scheme) == IS_STRING) {

                if (strcmp(Z_STRVAL(scheme), "http") == 0 || strcmp(Z_STRVAL(scheme), "https") == 0) {
                    if (Z_TYPE(port) == IS_NULL && strcmp(Z_STRVAL(scheme), "https") == 0) {
                        _port = 443;
                    } else if (Z_TYPE(port) == IS_LONG) {
                        _port = Z_LVAL(port);
                    }

                    span->setPeer(std::string(Z_STRVAL(host)) + ":" + std::to_string(_port));
                    span->setOperationName(Z_TYPE(path) == IS_NULL ? "/" : std::string(Z_STRVAL(path)));
                    span->addTag("url", std::string(Z_STRVAL(to_string)));

                    // with header
                    std::string header = segment->createHeader(span);
                    zval name, value;
                    ZVAL_STRING(&name, "sw8");
                    array_init(&value);
                    add_index_string(&value, 0, header.c_str());

                    zval *headers = sky_read_property(request, "headers", 0);
                    zval *headerNames = sky_read_property(request, "headerNames", 0);
                    add_assoc_zval_ex(headers, ZEND_STRL("sw8"), &value);
                    add_assoc_zval_ex(headerNames, ZEND_STRL("sw8"), &name);
                    ori_execute_ex(execute_data);
                    span->setEndTIme();
                    return span;
                }
            }
        }
    }

    return nullptr;
}