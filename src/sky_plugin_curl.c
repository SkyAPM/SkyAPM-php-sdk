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


#include "sky_plugin_curl.h"
#include <stdio.h>
#include <stdlib.h>
#include "sky_utils.h"
#include "php_skywalking.h"
#include "ext/standard/url.h"

void (*origin_curl_exec)(INTERNAL_FUNCTION_PARAMETERS) = NULL;

void (*origin_curl_setopt)(INTERNAL_FUNCTION_PARAMETERS) = NULL;

void (*origin_curl_setopt_array)(INTERNAL_FUNCTION_PARAMETERS) = NULL;

void (*origin_curl_close)(INTERNAL_FUNCTION_PARAMETERS) = NULL;

void sky_curl_setopt_handler(INTERNAL_FUNCTION_PARAMETERS) {

    sky_core_segment_t *segment = sky_util_find_segment_idx(execute_data, -1);

    if (segment == NULL || segment->isSkip) {
        origin_curl_setopt(INTERNAL_FUNCTION_PARAM_PASSTHRU);
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
    } else if (CURLOPT_HTTPHEADER == options && Z_TYPE_P(zvalue) == IS_ARRAY) {
        zval dup_header;
        ZVAL_DUP(&dup_header, zvalue);
        add_index_zval(&SKYWALKING_G(curl_cache), cid, &dup_header);
    }

    origin_curl_setopt(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void sky_curl_setopt_array_handler(INTERNAL_FUNCTION_PARAMETERS) {

    sky_core_segment_t *segment = sky_util_find_segment_idx(execute_data, -1);

    if (segment == NULL || segment->isSkip) {
        origin_curl_setopt_array(INTERNAL_FUNCTION_PARAM_PASSTHRU);
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

    if (http_header != NULL) {
        zval copy_header;
        ZVAL_DUP(&copy_header, http_header);
        add_index_zval(&SKYWALKING_G(curl_cache), cid, &copy_header);
    }

    origin_curl_setopt_array(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

void sky_curl_exec_handler(INTERNAL_FUNCTION_PARAMETERS) {
    sky_core_segment_t *segment = sky_util_find_segment_idx(execute_data, -1);

    if (segment == NULL || segment->isSkip) {
        origin_curl_exec(INTERNAL_FUNCTION_PARAM_PASSTHRU);
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
    call_user_function(CG(function_table), NULL, &func, &url_info, 1, args);
    zval_dtor(&func);
    zval_dtor(&args[0]);

    // check
    php_url *url_parse = NULL;
    zval *z_url = zend_hash_str_find(Z_ARRVAL(url_info), ZEND_STRL("url"));
    char *url_str = Z_STRVAL_P(z_url);
    if (strlen(url_str) > 0 && (starts_with("http://", url_str) || starts_with("https://", url_str))) {
        url_parse = php_url_parse(url_str);
        if (url_parse != NULL && url_parse->scheme != NULL && url_parse->host != NULL) {
            is_record = 1;
        }
    }

    // set header
    int is_malloc = 0;
    zval *header = zend_hash_index_find(Z_ARRVAL_P(&SKYWALKING_G(curl_cache)), cid);
    sky_core_span_t *span = NULL;
    if (is_record) {
        if (header == NULL) {
            header = (zval *) emalloc(sizeof(zval));
            bzero(header, sizeof(zval));
            array_init(header);
            is_malloc = 1;
        }

        // for php7.3.0+
#if PHP_VERSION_ID >= 70300
        char *php_url_scheme = ZSTR_VAL(url_parse->scheme);
        char *php_url_host = ZSTR_VAL(url_parse->host);
        char *php_url_path = url_parse->path != NULL ? ZSTR_VAL(url_parse->path) : NULL;
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

        span = sky_core_span_new(Exit, Http, 8002);
        char *peer = (char *) emalloc(sizeof(php_url_host) + 6);
        sprintf(peer, "%s:%d", php_url_host, peer_port);
        sky_core_span_set_peer(span, peer);
        sky_core_span_set_operation_name(span, php_url_path == NULL ? "/" : php_url_path);
        sky_core_span_add_tag(span, sky_core_tag_new("url", url_str));

//        std::string sw_header = segment->createHeader(span);
//        add_next_index_string(option, ("sw8: " + sw_header).c_str());

        zval argv[3];
        zval ret;
        ZVAL_STRING(&func, "curl_setopt");
        ZVAL_COPY(&argv[0], zid);
        ZVAL_LONG(&argv[1], SKY_CURLOPT_HTTPHEADER);
        ZVAL_COPY(&argv[2], header);
        call_user_function(CG(function_table), NULL, &func, &ret, 3, argv);
        zval_dtor(&func);
        zval_dtor(&ret);
        zval_dtor(&argv[0]);
        zval_dtor(&argv[1]);
        zval_dtor(&argv[2]);
        if (is_malloc) {
            zval_ptr_dtor(header);
            efree(header);
        }
    }

    origin_curl_exec(INTERNAL_FUNCTION_PARAM_PASSTHRU);

    // record
    if (is_record == 1) {

        // get response
        zval url_response;
        ZVAL_COPY(&args[0], zid);
        ZVAL_STRING(&func, "curl_getinfo");
        call_user_function(CG(function_table), NULL, &func, &url_response, 1, args);
        zval_dtor(&func);
        zval_dtor(&args[0]);

        zval *response_http_code = zend_hash_str_find(Z_ARRVAL(url_response), ZEND_STRL("http_code"));
        char code[255] = {0};
        sprintf(code, "%lld", Z_LVAL_P(response_http_code));
        sky_core_span_add_tag(span, sky_core_tag_new("status_code", code));
        if (Z_LVAL_P(response_http_code) == 0) {
            // get errors
            zval curl_error;
            ZVAL_COPY(&args[0], zid);
            ZVAL_STRING(&func, "curl_error");
            call_user_function(CG(function_table), NULL, &func, &curl_error, 1, args);

//            span->addLog("CURL_ERROR", Z_STRVAL(curl_error));
            sky_core_span_set_error(span, true);

            zval_dtor(&func);
            zval_dtor(&args[0]);
            zval_dtor(&curl_error);
        } else if (Z_LVAL_P(response_http_code) >= 400) {
            if (SKYWALKING_G(curl_response_enable) && Z_TYPE_P(return_value) == IS_STRING) {
                sky_core_span_add_tag(span, sky_core_tag_new("http.response", Z_STRVAL_P(return_value)));
            }

            sky_core_span_set_error(span, true);
        } else {
            sky_core_span_set_error(span, false);
        }
        zval_dtor(&url_response);

        sky_core_span_set_end_time(span);
    }

    zval_dtor(&url_info);
    if (url_parse != NULL) {
        php_url_free(url_parse);
    }
}

void sky_curl_close_handler(INTERNAL_FUNCTION_PARAMETERS) {

    sky_core_segment_t *segment = sky_util_find_segment_idx(execute_data, -1);

    if (segment == NULL || segment->isSkip) {
        origin_curl_close(INTERNAL_FUNCTION_PARAM_PASSTHRU);
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

    zval *http_header = zend_hash_index_find(Z_ARRVAL_P(&SKYWALKING_G(curl_cache)), cid);
    if (http_header != NULL) {
        zend_hash_index_del(Z_ARRVAL_P(&SKYWALKING_G(curl_cache)), cid);
    }

    origin_curl_close(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}