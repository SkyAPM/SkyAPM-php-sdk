/*
 * Copyright 2022 SkyAPM
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

#include "sky_core_module.h"
#include "php.h"
#include "SAPI.h"
#include "ext/standard/php_var.h"
#include "zend_hash.h"
#include "sys/ipc.h"

#include "php_skywalking.h"
#include "sky_core_segment.h"
#include "sky_core_span.h"
#include "sky_core_tag.h"
#include "sky_core_cross_process.h"
#include "sky_plugin_curl.h"
#include "sky_go_wrapper.h"
#include "sky_plugin_redis.h"

int le_skywalking_pconnect;

const char *skywalking_persistent_id = "skywalking_persistent_id";

extern void (*ori_execute_ex)(zend_execute_data *execute_data);

extern void (*ori_execute_internal)(zend_execute_data *execute_data, zval *return_value);

extern void (*origin_curl_exec)(INTERNAL_FUNCTION_PARAMETERS);

extern void (*origin_curl_setopt)(INTERNAL_FUNCTION_PARAMETERS);

extern void (*origin_curl_setopt_array)(INTERNAL_FUNCTION_PARAMETERS);

extern void (*origin_curl_close)(INTERNAL_FUNCTION_PARAMETERS);

int sky_core_module_init(INIT_FUNC_ARGS) {

    if (1 == 1) {
        sky_plugin_redis_hooks();
    }

    // todo if on
    if (1 == 1) {
        zend_function *origin_function;
        if ((origin_function = SKY_FIND_FUNC("curl_exec")) != NULL) {
            origin_curl_exec = origin_function->internal_function.handler;
            origin_function->internal_function.handler = sky_curl_exec_handler;
        }
        if ((origin_function = SKY_FIND_FUNC("curl_setopt")) != NULL) {
            origin_curl_setopt = origin_function->internal_function.handler;
            origin_function->internal_function.handler = sky_curl_setopt_handler;
        }
        if ((origin_function = SKY_FIND_FUNC("curl_setopt_array")) != NULL) {
            origin_curl_setopt_array = origin_function->internal_function.handler;
            origin_function->internal_function.handler = sky_curl_setopt_array_handler;
        }
        if ((origin_function = SKY_FIND_FUNC("curl_close")) != NULL) {
            origin_curl_close = origin_function->internal_function.handler;
            origin_function->internal_function.handler = sky_curl_close_handler;
        }
    }

    // register
    le_skywalking_pconnect = zend_register_list_destructors_ex(NULL, skywalking_dtor,
                                                               "skywalking persistent connection", module_number);

    HashTable *segments = pemalloc(sizeof(HashTable), 1);
    zend_hash_init(segments, 0, NULL, ZVAL_PTR_DTOR, 1);
    SKYWALKING_G(segments) = segments;

    // register
    skywalking_connect(SKYWALKING_G(grpc), SKYWALKING_G(app_code), SKYWALKING_G(instance_name));
    char *instance = skywalking_get_instance();

    skywalking_t *skywalking = (skywalking_t *) pecalloc(1, sizeof(skywalking_t), 1);
    strcpy(skywalking->serviceInstance, instance);
//    skywalking->connect = sky_connect;
#if PHP_VERSION_ID >= 70300
    zend_register_persistent_resource(skywalking_persistent_id, strlen(skywalking_persistent_id), skywalking,
                                      le_skywalking_pconnect);
#else
    zend_resource new_le;
    new_le.type = le_skywalking_pconnect;
    new_le.ptr = skywalking;
    zend_hash_str_update_mem(&EG(persistent_list), skywalking_persistent_id, strlen(skywalking_persistent_id), &new_le, sizeof(zend_resource));
#endif

    return 0;
}

static ZEND_RSRC_DTOR_FUNC(skywalking_dtor) {

};

void sky_core_module_free() {
//    pefree(SKYWALKING_G(segments), 1);
}

void sky_core_request_init(zval *request, u_int64_t request_id) {

    array_init(&SKYWALKING_G(curl_cache));

    zval *_server = NULL;
    zval *cross_process_protocol = NULL;
    char *operation_name = NULL;
    char *peer = NULL;

    if (request != NULL) {

    } else { // fpm or cli mode
        zend_bool jit = PG(auto_globals_jit);
        if (jit) {
            zend_string *server = zend_string_init(ZEND_STRL("_SERVER"), 0);
            zend_is_auto_global(server);
            zend_string_release(server);
        }
        _server = zend_hash_str_find(&EG(symbol_table), ZEND_STRL("_SERVER"));
        if (strcasecmp(SKYWALKING_G(cross_process_protocol), "3.0") == 0) {
            cross_process_protocol = zend_hash_str_find(Z_ARRVAL_P(_server), ZEND_STRL("HTTP_SW8"));
        }

        // name
        if (strcasecmp("cli", sapi_module.name) == 0) {
            operation_name = "cli";
        } else {
            zval *request_uri = zend_hash_str_find(Z_ARRVAL_P(_server), ZEND_STRL("REQUEST_URI"));
            operation_name = Z_STRVAL_P(request_uri);
        }

        // peer
        zval *http_host = zend_hash_str_find(Z_ARRVAL_P(_server), ZEND_STRL("HTTP_HOST"));
        zval *server_port = zend_hash_str_find(Z_ARRVAL_P(_server), ZEND_STRL("SERVER_PORT"));
        if (http_host == NULL) {
            http_host = zend_hash_str_find(Z_ARRVAL_P(_server), ZEND_STRL("SERVER_ADDR"));
        }
        if (http_host != NULL && server_port != NULL) {
            asprintf(&peer, "%s:%s", Z_STRVAL_P(http_host), Z_STRVAL_P(server_port));
        }
    }

    // new segment
    char *protocol = cross_process_protocol != NULL ? Z_STRVAL_P(cross_process_protocol) : NULL;
    sky_core_segment_t *core_segment = sky_core_segment_new(protocol);
    sky_core_span_t *span = sky_core_span_new(Entry, Http, 8001);
    sky_core_span_set_operation_name(span, operation_name);
    sky_core_span_set_peer(span, peer);

    sky_core_span_add_tag(span, sky_core_tag_new("url", operation_name));

    zval *request_method = zend_hash_str_find(Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]),
                                              ZEND_STRL("REQUEST_METHOD"));
    if (request_method != NULL) {
        sky_core_span_add_tag(span, sky_core_tag_new("http.method", Z_STRVAL_P(request_method)));
    }

    sky_core_segment_add_span(core_segment, span);

    zval segment;
    ZVAL_PTR(&segment, core_segment);
    zend_hash_index_add(SKYWALKING_G(segments), request_id, &segment);
}

void sky_core_request_free(zval *response, u_int64_t request_id) {
    zval_dtor(&SKYWALKING_G(curl_cache));

    if (SKYWALKING_G(segments) == NULL) {
        return;
    }

    zval *segment = zend_hash_index_find(SKYWALKING_G(segments), request_id);
    sky_core_segment_t *core_segment = (sky_core_segment_t *) Z_PTR_P(segment);

    if (response == NULL) {
//        segment->setStatusCode(SG(sapi_headers).http_response_code);
    }
    sky_core_span_set_end_time(core_segment->spans[0]);


    zend_string *persistent_id = zend_string_init(skywalking_persistent_id, strlen(skywalking_persistent_id), 0);
    zend_resource *le = zend_hash_find_ptr(&EG(persistent_list), persistent_id);
    zend_string_release(persistent_id);
    skywalking_t *skywalking = (skywalking_t *) le->ptr;
    sky_core_segment_set_service(core_segment, skywalking->service);
    sky_core_segment_set_service_instance(core_segment, skywalking->serviceInstance);

    char *json = sky_core_segment_to_json(core_segment);
    skywalking_write_segment(skywalking->connect, json);

    zend_hash_index_del(SKYWALKING_G(segments), request_id);
//    efree(segment);
    free(json);
}