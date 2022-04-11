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


#include <regex>
#include "sky_pdo.h"
#include "sky_utils.h"

#include "sky_core_segment.h"

#include "php_skywalking.h"

SkyCoreSpan *sky_pdo(zend_execute_data *execute_data, const std::string &class_name, const std::string &function_name) {

    if (class_name == "PDO") {
        if (function_name == "exec" || function_name == "query" ||
            function_name == "prepare" || function_name == "commit" ||
            function_name == "begintransaction" || function_name == "rollback") {
            auto *segment = sky_get_segment(execute_data, -1);
            if (segment->skip()) {
                return nullptr;
            }

            auto *span = segment->createSpan(SkyCoreSpanType::Exit, SkyCoreSpanLayer::Database, 8003);
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
    } else {
        if (function_name == "execute") {
            auto *segment = sky_get_segment(execute_data, -1);
            if (segment->skip()) {
                return nullptr;
            }

            auto *span = segment->createSpan(SkyCoreSpanType::Exit, SkyCoreSpanLayer::Database, 8003);
            span->setOperationName(class_name + "->" + function_name);

            span->setPeer(sky_pdo_statement_peer(span, execute_data));

            return span;
        }
    }


    return nullptr;
}

std::string sky_pdo_statement_peer(SkyCoreSpan *span, zend_execute_data *execute_data) {
    pdo_stmt_t *stmt = (pdo_stmt_t *) Z_PDO_STMT_P(&(execute_data->This));

    if (stmt != nullptr) {
#if PHP_VERSION_ID >= 80100
        span->addTag("db.statement", ZSTR_VAL(stmt->query_string));
#else
        span->addTag("db.statement", stmt->query_string);
#endif

        if (stmt->dbh != nullptr) {
            return sky_pdo_dbh_peer(span, stmt->dbh);
        }
    }

    return nullptr;
}

std::string sky_pdo_peer(SkyCoreSpan *span, zend_execute_data *execute_data) {
    pdo_dbh_t *dbh = Z_PDO_DBH_P(&(execute_data)->This);

    if (dbh != nullptr) {
        return sky_pdo_dbh_peer(span, dbh);
    }

    return nullptr;
}

std::string sky_pdo_dbh_peer(SkyCoreSpan *span, pdo_dbh_t *dbh) {
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

        for (auto item:items) {
            std::vector<std::string> kv(std::sregex_token_iterator(item.begin(), item.end(), kv_re, -1), std::sregex_token_iterator());
            if (kv.size() >= 2) {
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
    return nullptr;
}

void sky_pdo_check_errors(zend_execute_data *execute_data, SkyCoreSpan *span) {
    zval *obj = &(execute_data)->This, property, return_ptr;
    ZVAL_STRING(&property, "errorInfo");
    
    call_user_function(CG(function_table), obj, &property, &return_ptr, 0, nullptr);
    if (Z_TYPE(return_ptr) == IS_ARRAY) {
        span->pushLog(new SkyCoreLog("SQLSTATE", Z_STRVAL_P(zend_hash_index_find(Z_ARRVAL(return_ptr), 0))));
        span->pushLog(new SkyCoreLog("Error Code", std::to_string(Z_LVAL_P(zend_hash_index_find(Z_ARRVAL(return_ptr), 1)))));
        span->pushLog(new SkyCoreLog("Error", Z_STRVAL_P(zend_hash_index_find(Z_ARRVAL(return_ptr), 2))));
    }

    zval_dtor(&return_ptr);
    zval_dtor(&property);
}
