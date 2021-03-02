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

#include "sky_plugin_mysqli.h"
#include "sky_core_span_log.h"
#include "segment.h"
#include "sky_utils.h"
#include "php_skywalking.h"

#include "ext/mysqli/php_mysqli_structs.h"

void sky_mysqli_peer(Span *span, mysqli_object *mysqli) {
    MYSQLI_RESOURCE *my_res = (MYSQLI_RESOURCE *) mysqli->ptr;
    if (my_res && my_res->ptr) {
        MY_MYSQL *mysql = (MY_MYSQL *) my_res->ptr;
        if (mysql->mysql) {
#if PHP_VERSION_ID >= 70100
            std::string host = mysql->mysql->data->hostname.s;
#else
            std::string host = mysql->mysql->data->host;
#endif
            span->addTag("db.type", "mysql");
            span->setPeer(host + ":" + std::to_string(mysql->mysql->data->port));
        }
    }
}

Span *sky_plugin_mysqli(zend_execute_data *execute_data, const std::string &class_name, const std::string &function_name) {
    mysqli_object *mysqli = nullptr;
    if (function_name == "query" || function_name == "autocommit" || function_name == "commit" || function_name == "rollback" ||
        function_name == "mysqli_query" || function_name == "mysqli_autocommit" || function_name == "mysqli_commit" || function_name == "mysqli_rollback") {
            auto *segment = sky_get_segment(execute_data, -1);
            auto *span = segment->createSpan(SkySpanType::Exit, SkySpanLayer::Database, 8004);
            uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data);

            zval *statement = nullptr;
            if (class_name == "mysqli") {
                span->setOperationName(class_name + "->" + function_name);
                if (arg_count) {
                    statement = ZEND_CALL_ARG(execute_data, 1);
                }
                mysqli = (mysqli_object *) Z_MYSQLI_P(&(execute_data->This));
            } else { //is procedural
                span->setOperationName(function_name);
                if (arg_count > 1) {
                    statement = ZEND_CALL_ARG(execute_data, 2);
                }
                zval *obj = ZEND_CALL_ARG(execute_data, 1);
                if  (Z_TYPE_P(obj) != IS_NULL){
                    mysqli = (mysqli_object *) Z_MYSQLI_P(obj);
                }
            }

            if (statement != nullptr && Z_TYPE_P(statement) == IS_STRING) {
                span->addTag("db.statement", Z_STRVAL_P(statement));
            }
            if (mysqli != nullptr){
                sky_mysqli_peer(span, mysqli);
            }

            return span;
    }

    return nullptr;
}

void sky_plugin_mysqli_check_errors(zend_execute_data *execute_data, Span *span, int is_oop) { 
    zval *obj, rv;
    if (is_oop == 1) {
        obj = &(execute_data)->This;
    } else {
        obj = ZEND_CALL_ARG(execute_data, 1);
    }
    
#if PHP_VERSION_ID < 80000
        zend_read_property(obj->value.obj->ce, obj, ZEND_STRL("error_list"), 0, &rv);
#else
        zend_read_property(obj->value.obj->ce, obj->value.obj, ZEND_STRL("error_list"), 0, &rv);
#endif

    zend_string *key;
    zval *value, *item;
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(&rv), item) {
        if (Z_TYPE_P(item) == IS_ARRAY) {
            ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(item), key, value) {
                if (Z_TYPE_P(value) == IS_LONG) {
                    span->addLog(key->val, std::to_string(Z_LVAL_P(value)));
                } else {
                    span->addLog(key->val, Z_STRVAL_P(value));
                }
            } ZEND_HASH_FOREACH_END();
        }
    }ZEND_HASH_FOREACH_END();

    zval_dtor(&rv);
}
