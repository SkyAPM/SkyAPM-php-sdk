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
#include "sky_mysqli.h"

#include "segment.h"

#include "php_skywalking.h"

#include "ext/mysqli/php_mysqli_structs.h"


Span *sky_mysqli(zend_execute_data *execute_data, const std::string &class_name, const std::string &function_name) {
    mysqli_object *mysqli = nullptr;
    if (class_name == "mysqli") {
        if (function_name == "query" || function_name == "commit" || function_name == "rollback") {            
            auto *segment = static_cast<Segment *>(SKYWALKING_G(segment));
            auto *span = segment->createSpan(SkySpanType::Exit, SkySpanLayer::Database, 8004);
            span->setOperationName(class_name + "->" + function_name);

            uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);
            if (arg_count) {
                zval *statement = ZEND_CALL_ARG(execute_data, 1);
                if (Z_TYPE_P(statement) == IS_STRING) {
                    span->addTag("db.statement", Z_STRVAL_P(statement));
                }
            }

            mysqli = (mysqli_object *) Z_MYSQLI_P(&(execute_data->This));
            span->setPeer(sky_mysqli_peer(span, execute_data, mysqli));

            return span;
        }
    } else {                     
        if (function_name == "mysqli_query") {
            auto *segment = static_cast<Segment *>(SKYWALKING_G(segment));
            auto *span = segment->createSpan(SkySpanType::Exit, SkySpanLayer::Database, 8004);
            span->setOperationName("mysqli->query");

            zval *statement = ZEND_CALL_ARG(execute_data, 2);
            if (Z_TYPE_P(statement) == IS_STRING) {
                span->addTag("db.statement", Z_STRVAL_P(statement));
            }

            mysqli = (mysqli_object *) Z_MYSQLI_P(ZEND_CALL_ARG(execute_data, 1));   
            span->setPeer(sky_mysqli_peer(span, execute_data, mysqli));

            return span;
        }
    }

    return nullptr;
}

std::string sky_mysqli_peer(Span *span, zend_execute_data *execute_data, mysqli_object *mysqli) {
    MYSQLI_RESOURCE *my_res = (MYSQLI_RESOURCE *) mysqli->ptr;
    if (my_res && my_res->ptr) {
        MY_MYSQL *mysql = (MY_MYSQL *) my_res->ptr;
        if (mysql->mysql) {
#if PHP_VERSION_ID >= 70100
            char *host = mysql->mysql->data->hostname.s;
#else
            char *host = mysql->mysql->data->host;
#endif
            span->addTag("db.type", "mysql");
            char *peer = (char *) emalloc(strlen(host) + 6);
            sprintf(peer, "%s:%d", host, mysql->mysql->data->port);

            return peer;
        }
    }

    return nullptr;
}