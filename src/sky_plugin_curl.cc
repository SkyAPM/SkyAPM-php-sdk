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

#include <iostream>
#include "sky_plugin_curl.h"
#include "php_skywalking.h"

#include "segment.h"
#include "sky_utils.h"

void (*orig_curl_exec)(INTERNAL_FUNCTION_PARAMETERS) = nullptr;

void (*orig_curl_setopt)(INTERNAL_FUNCTION_PARAMETERS) = nullptr;

void (*orig_curl_setopt_array)(INTERNAL_FUNCTION_PARAMETERS) = nullptr;

void (*orig_curl_close)(INTERNAL_FUNCTION_PARAMETERS) = nullptr;

void sky_curl_setopt_handler(INTERNAL_FUNCTION_PARAMETERS) {

    auto *segment = sky_get_segment(execute_data, -1);
    if (segment == nullptr) {
        orig_curl_setopt(INTERNAL_FUNCTION_PARAM_PASSTHRU);
        return;
    }

    zval *zid, *zvalue;
    zend_long options;
#if PHP_VERSION_ID >= 80000
    ZEND_PARSE_PARAMETERS_START(3, 3)
            Z_PARAM_OBJECT_OF_CLASS(zid, curl_ce)
            Z_PARAM_LONG(options)
            Z_PARAM_ZVAL(zvalue)
    ZEND_PARSE_PARAMETERS_END();
    zend_ulong cid = Z_OBJ_HANDLE_P(zid);
#else
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rlz", &zid, &options, &zvalue) == FAILURE) {
        return;
    }
    zend_ulong cid = Z_RES_HANDLE_P(zid);
#endif

    if (SKY_CURLOPT_HTTPHEADER == options) {
        zval *header = ZEND_CALL_ARG(execute_data, 2);
        header->value.lval = CURLOPT_HTTPHEADER;
        orig_curl_setopt(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    } else {
        if (CURLOPT_HTTPHEADER == options && Z_TYPE_P(zvalue) == IS_ARRAY) {
            zval dup_header;
            ZVAL_DUP(&dup_header, zvalue);
            add_index_zval(&SKYWALKING_G(curl_header), cid, &dup_header);
        } else {
            orig_curl_setopt(INTERNAL_FUNCTION_PARAM_PASSTHRU);
        }
    }
}

void sky_curl_setopt_array_handler(INTERNAL_FUNCTION_PARAMETERS) {

    auto *segment = sky_get_segment(execute_data, -1);
    if (segment == nullptr) {
        orig_curl_setopt_array(INTERNAL_FUNCTION_PARAM_PASSTHRU);
        return;
    }

    zval *zid, *arr;
#if PHP_VERSION_ID >= 80000
    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_OBJECT_OF_CLASS(zid, curl_ce)
            Z_PARAM_ARRAY(arr)
    ZEND_PARSE_PARAMETERS_END();
    zend_ulong cid = Z_OBJ_HANDLE_P(zid);
#else
    ZEND_PARSE_PARAMETERS_START(2, 2)
            Z_PARAM_RESOURCE(zid)
            Z_PARAM_ARRAY(arr)
    ZEND_PARSE_PARAMETERS_END();
    zend_ulong cid = Z_RES_HANDLE_P(zid);
#endif

    zval *http_header = zend_hash_index_find(Z_ARRVAL_P(arr), CURLOPT_HTTPHEADER);

    if (http_header != nullptr) {
        zval copy_header;
        ZVAL_DUP(&copy_header, http_header);
        add_index_zval(&SKYWALKING_G(curl_header), cid, &copy_header);
    }

    orig_curl_setopt_array(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void sky_curl_exec_handler(INTERNAL_FUNCTION_PARAMETERS) {
    auto *segment = sky_get_segment(execute_data, -1);
    if (segment == nullptr) {
        orig_curl_exec(INTERNAL_FUNCTION_PARAM_PASSTHRU);
        return;
    }

    zval *zid;

#if PHP_VERSION_ID >= 80000
    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_OBJECT_OF_CLASS(zid, curl_ce)
    ZEND_PARSE_PARAMETERS_END();
    zend_ulong cid = Z_OBJ_HANDLE_P(zid);
#else
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zid) == FAILURE) {
        return;
    }
    zend_ulong cid = Z_RES_HANDLE_P(zid);
#endif

    int is_record = 0;

    zval func;
    zval args[1];
    zval url_info;
    ZVAL_COPY(&args[0], zid);
    ZVAL_STRING(&func, "curl_getinfo");
    call_user_function(CG(function_table), nullptr, &func, &url_info, 1, args);
    zval_dtor(&func);
    zval_dtor(&args[0]);

    // check
    php_url *url_parse = nullptr;
    zval *z_url = zend_hash_str_find(Z_ARRVAL(url_info), ZEND_STRL("url"));
    char *url_str = Z_STRVAL_P(z_url);
    if (strlen(url_str) > 0 && (starts_with("http://", url_str) || starts_with("https://", url_str))) {
        url_parse = php_url_parse(url_str);
        if (url_parse != nullptr && url_parse->scheme != nullptr && url_parse->host != nullptr) {
            is_record = 1;
        }
    }

    // set header
    Span *span;
    int is_emalloc = 0;
    zval *option;
    option = zend_hash_index_find(Z_ARRVAL_P(&SKYWALKING_G(curl_header)), cid);
    if (is_record) {
        if (option == nullptr) {
            option = (zval *) emalloc(sizeof(zval));
            bzero(option, sizeof(zval));
            array_init(option);
            is_emalloc = 1;
        }

        // for php7.3.0+
#if PHP_VERSION_ID >= 70300
        char *php_url_scheme = ZSTR_VAL(url_parse->scheme);
        char *php_url_host = ZSTR_VAL(url_parse->host);
        char *php_url_path = url_parse->path != nullptr ? ZSTR_VAL(url_parse->path) : nullptr;
#else
        char *php_url_scheme = url_parse->scheme;
        char *php_url_host = url_parse->host;
        char *php_url_path = url_parse->path;
#endif

        int peer_port;
        if (url_parse->port) {
            peer_port = url_parse->port;
        } else {
            if (strcasecmp("http", php_url_scheme) == 0) {
                peer_port = 80;
            } else {
                peer_port = 443;
            }
        }

        span = segment->createSpan(SkySpanType::Exit, SkySpanLayer::Http, 8002);
        span->setPeer(std::string(php_url_host) + ":" + std::to_string(peer_port));
        span->setOperationName(php_url_path == nullptr ? "/" : php_url_path);
        span->addTag("url", url_str);

        std::string sw_header = segment->createHeader(span);
        add_next_index_string(option, ("sw8: " + sw_header).c_str());

        zval argv[3];
        zval ret;
        ZVAL_STRING(&func, "curl_setopt");
        ZVAL_COPY(&argv[0], zid);
        ZVAL_LONG(&argv[1], SKY_CURLOPT_HTTPHEADER);
        ZVAL_COPY(&argv[2], option);
        call_user_function(CG(function_table), nullptr, &func, &ret, 3, argv);
        zval_dtor(&func);
        zval_dtor(&ret);
        zval_dtor(&argv[0]);
        zval_dtor(&argv[1]);
        zval_dtor(&argv[2]);
        if (is_emalloc) {
            zval_ptr_dtor(option);
            efree(option);
        }
    }

    orig_curl_exec(INTERNAL_FUNCTION_PARAM_PASSTHRU);

    // record
    if (is_record == 1) {

        // get response
        zval url_response;
        ZVAL_COPY(&args[0], zid);
        ZVAL_STRING(&func, "curl_getinfo");
        call_user_function(CG(function_table), nullptr, &func, &url_response, 1, args);
        zval_dtor(&func);
        zval_dtor(&args[0]);

        zval *response_http_code;
        response_http_code = zend_hash_str_find(Z_ARRVAL(url_response), ZEND_STRL("http_code"));
        span->addTag("status_code", std::to_string(Z_LVAL_P(response_http_code)));
        if (Z_LVAL_P(response_http_code) == 0) {
            // get errors
            zval curl_error;
            ZVAL_COPY(&args[0], zid);
            ZVAL_STRING(&func, "curl_error");
            call_user_function(CG(function_table), nullptr, &func, &curl_error, 1, args);
                      
            span->addLog("CURL_ERROR", Z_STRVAL(curl_error));
            span->setIsError(true);

            zval_dtor(&func);
            zval_dtor(&args[0]);
            zval_dtor(&curl_error);
        } else if (Z_LVAL_P(response_http_code) >= 400) {
            // TODO: response body set to logs
            span->setIsError(true);
        } else {
            span->setIsError(false);
        }
        zval_dtor(&url_response);

        span->setEndTIme();
    }

    zval_dtor(&url_info);
    if (url_parse != nullptr) {
        php_url_free(url_parse);
    }
}

void sky_curl_close_handler(INTERNAL_FUNCTION_PARAMETERS) {

    auto *segment = sky_get_segment(execute_data, -1);
    if (segment == nullptr) {
        orig_curl_close(INTERNAL_FUNCTION_PARAM_PASSTHRU);
        return;
    }

    zval *zid;

#if PHP_VERSION_ID >= 80000
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_OBJECT_OF_CLASS(zid, curl_ce)
    ZEND_PARSE_PARAMETERS_END();
    zend_ulong cid = Z_OBJ_HANDLE_P(zid);
#else
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zid) == FAILURE) {
        return;
    }
    zend_ulong cid = Z_RES_HANDLE_P(zid);
#endif

    zval *http_header = zend_hash_index_find(Z_ARRVAL_P(&SKYWALKING_G(curl_header)), cid);
    if (http_header != nullptr) {
        zend_hash_index_del(Z_ARRVAL_P(&SKYWALKING_G(curl_header)), cid);
    }

    orig_curl_close(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}