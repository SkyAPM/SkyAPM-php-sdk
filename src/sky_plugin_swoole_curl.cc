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
#include "sky_plugin_swoole_curl.h"
#include "sky_core_segment.h"
#include "php_skywalking.h"
#include "sky_utils.h"

extern void (*ori_execute_ex)(zend_execute_data *execute_data);

void sky_plugin_swoole_curl(zend_execute_data *execute_data, const std::string &class_name, const std::string &function_name) {
    uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);

    if (arg_count >= 1) {
        SkyCoreSpan *span = nullptr;

        zval *handle = ZEND_CALL_ARG(execute_data, 1);

        zval *urlInfo = sky_read_property(handle, "urlInfo", 0);
        zval *scheme = zend_hash_str_find(Z_ARRVAL_P(urlInfo), ZEND_STRL("scheme"));
        zval *host = zend_hash_str_find(Z_ARRVAL_P(urlInfo), ZEND_STRL("host"));
        zval *port = zend_hash_str_find(Z_ARRVAL_P(urlInfo), ZEND_STRL("port"));
        zval *path = zend_hash_str_find(Z_ARRVAL_P(urlInfo), ZEND_STRL("path"));

        if (!Z_ISUNDEF_P(scheme) && Z_TYPE_P(scheme) == IS_STRING) {
            std::string _scheme(Z_STRVAL_P(scheme));
            std::string _host(Z_STRVAL_P(host));
            int _port(Z_LVAL_P(port));
            std::string _path;
            std::string _url;

            zval url;
            SKY_ZEND_CALL_METHOD(handle, nullptr, "geturl", &url, 0, nullptr, nullptr);

            if (!Z_ISUNDEF(url) && Z_TYPE(url) == IS_STRING) {
                _url = std::string(Z_STRVAL(url));
            }

            if (path != nullptr && !Z_ISUNDEF_P(path) && Z_TYPE_P(path) == IS_STRING) {
                _path = std::string(Z_STRVAL_P(path));
            } else {
                _path = "/";
            }

            if (_scheme == "http" || _scheme == "https") {
                auto *segment = sky_get_segment(execute_data, -1);
                if (segment != nullptr) {
                    span = segment->createSpan(SkyCoreSpanType::Exit, SkyCoreSpanLayer::Http, 8002);
                    span->setPeer(_host + ":" + std::to_string(_port));
                    span->setOperationName(_path);
                    span->addTag("url", _scheme + "://" + _host + ":" + std::to_string(_port) + _url);
                    std::string header = segment->createHeader(span);

                    zval opt, value;
                    ZVAL_LONG(&opt, CURLOPT_HTTPHEADER);
                    array_init(&value);
                    add_index_string(&value, 0, ("sw8: " + header).c_str());

                    SKY_ZEND_CALL_METHOD(handle, nullptr, "setopt", nullptr, 2, &opt, &value);
                }
            }
        }

        ori_execute_ex(execute_data);

        if (span) {
            zval *info = sky_read_property(handle, "info", 0);
            zval *http_code = zend_hash_str_find(Z_ARRVAL_P(info), ZEND_STRL("http_code"));

            if (!Z_ISUNDEF_P(http_code) && Z_TYPE_P(http_code) == IS_LONG) {
                span->addTag("status_code", std::to_string(Z_LVAL_P(http_code)));
                if (Z_LVAL_P(http_code) >= 400) {
                    span->setIsError(true);
                }
            }
            span->setEndTIme();
        }
    } else {
        ori_execute_ex(execute_data);
    }
}