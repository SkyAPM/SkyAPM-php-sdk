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

#include <regex>
#include "sky_pdo.h"

#include "segment.h"

#include "php_skywalking.h"

Span *sky_pdo(zend_execute_data *execute_data, const std::string &class_name, const std::string &function_name) {

    if (function_name == "exec" || function_name == "query" ||
        function_name == "prepare" || function_name == "commit" ||
        function_name == "begintransaction" || function_name == "rollback") {
        auto *segment = static_cast<Segment *>(SKYWALKING_G(segment));
        auto *span = segment->createSpan(SkySpanType::Exit, SkySpanLayer::Database, 8003);
        span->setOperationName(class_name + "->" + function_name);

        uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
        if (arg_count) {
            // todo statement
            zval *statement = ZEND_CALL_ARG(execute_data, 1);
            if (Z_TYPE_P(statement) == IS_STRING) {
                span->addTag("db.statement", Z_STRVAL_P(statement));
            }
        }

        span->setPeer(sky_pdo_peer(span, execute_data));

        return span;
    }

    return nullptr;
}

static std::string sky_pdo_peer(Span *span, zend_execute_data *execute_data) {
    pdo_dbh_t *dbh = Z_PDO_DBH_P(&(execute_data)->This);

    if (dbh != nullptr) {

        if (dbh->driver != nullptr) {
            span->addTag("db.type", dbh->driver->driver_name);
        }

        if (dbh->data_source != nullptr) {
            span->addTag("db.data_source", dbh->data_source);
            std::regex ws_re(";");
            std::regex kv_re("=");
            std::string source(dbh->data_source);
            std::vector<std::string> items(std::sregex_token_iterator(source.begin(), source.end(), ws_re, -1), std::sregex_token_iterator());

            std::string host("not_found");
            std::string port("3306");

            for(auto item:items) {
                std::vector<std::string> kv(std::sregex_token_iterator(item.begin(), item.end(), kv_re, -1), std::sregex_token_iterator());
                if (kv.size() >=2) {
                    if (kv[0] == "host") {
                        host = kv[1];
                    }
                    if (kv[0] == "port") {
                        port = kv[1];
                    }
                }
            }

            return host + ":" + port;
        }
    }

    return nullptr;
}