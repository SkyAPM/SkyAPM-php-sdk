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
#include "sky_util_php.h"
#include "php_skywalking.h"
#include "ext/standard/url.h"

zif_handler origin_curl_exec;

zif_handler origin_curl_setopt;

zif_handler origin_curl_setopt_array;

zif_handler origin_curl_close;

void sky_plugin_curl_hooks() {
    zend_function *origin_function;
    if ((origin_function = sky_util_find_func("curl_exec")) != NULL) {
        origin_curl_exec = origin_function->internal_function.handler;
        origin_function->internal_function.handler = sky_curl_exec_handler;
    }
    if ((origin_function = sky_util_find_func("curl_setopt")) != NULL) {
        origin_curl_setopt = origin_function->internal_function.handler;
        origin_function->internal_function.handler = sky_curl_setopt_handler;
    }
    if ((origin_function = sky_util_find_func("curl_setopt_array")) != NULL) {
        origin_curl_setopt_array = origin_function->internal_function.handler;
        origin_function->internal_function.handler = sky_curl_setopt_array_handler;
    }
    if ((origin_function = sky_util_find_func("curl_close")) != NULL) {
        origin_curl_close = origin_function->internal_function.handler;
        origin_function->internal_function.handler = sky_curl_close_handler;
    }
}

ZEND_NAMED_FUNCTION(sky_curl_setopt_handler) {

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

ZEND_NAMED_FUNCTION(sky_curl_setopt_array_handler) {

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

ZEND_NAMED_FUNCTION(sky_curl_exec_handler) {
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

    zval params[5];
    ZVAL_COPY(&params[0], zid);
    zval url_info;
    sky_util_call_user_func("curl_getinfo", &url_info, 1, params);

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
        efree(peer);
        sky_core_span_set_operation_name(span, php_url_path == NULL ? "/" : php_url_path);
        sky_core_span_add_tag(span, sky_core_tag_new("url", url_str));

//        std::string sw_header = segment->createHeader(span);
//        add_next_index_string(option, ("sw8: " + sw_header).c_str());

        ZVAL_COPY(&params[0], zid);
        ZVAL_LONG(&params[1], SKY_CURLOPT_HTTPHEADER);
        ZVAL_COPY(&params[2], header);
        zval set_result;
        sky_util_call_user_func("curl_setopt", &set_result, 3, params);
        zval_dtor(&set_result);

        if (is_malloc) {
            zval_ptr_dtor(header);
            efree(header);
        }
    }

    origin_curl_exec(INTERNAL_FUNCTION_PARAM_PASSTHRU);

    // record
    if (is_record == 1) {

        // get response
        ZVAL_COPY(&params[0], zid);
        zval url_response;
        sky_util_call_user_func("curl_getinfo", &url_response, 1, params);

        zval *response_http_code = zend_hash_str_find(Z_ARRVAL(url_response), ZEND_STRL("http_code"));
        char code[255] = {0};
        sprintf(code, "%ld", Z_LVAL_P(response_http_code));
        sky_core_span_add_tag(span, sky_core_tag_new("status_code", code));

        if (Z_LVAL_P(response_http_code) == 0) {
            // get errors
            ZVAL_COPY(&params[0], zid);
            zval curl_error;
            sky_util_call_user_func("curl_error", &curl_error, 1, params);
//            span->addLog("CURL_ERROR", Z_STRVAL(curl_error));
            sky_core_span_set_error(span, true);
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
        sky_core_segment_add_span(segment, span);
    }

    zval_dtor(&url_info);
    if (url_parse != NULL) {
        php_url_free(url_parse);
    }
}

ZEND_NAMED_FUNCTION(sky_curl_close_handler) {

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