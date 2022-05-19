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
#include "pthread.h"

#include "php_skywalking.h"
#include "sky_core_segment.h"
#include "sky_core_span.h"
#include "sky_core_tag.h"
#include "sky_plugin_curl.h"
#include "sky_core_report.h"
#include "sky_plugin_redis.h"
#include "sys/mman.h"

void *thread_sky_core_report_new(void *argv) {
//    report_new_t *args = (report_new_t *) argv;
    sky_core_report_new(
            SKYWALKING_G(grpc_address),
            SKYWALKING_G(service),
            SKYWALKING_G(real_service_instance),
            SKYWALKING_G(log_level),
            SKYWALKING_G(log_path)
    );
}

void delete_segments(zval *zv) {
    sky_core_segment_t *t = Z_PTR_P(zv);
    efree(t);
}

int sky_core_module_init(INIT_FUNC_ARGS) {

    if (strlen(SKYWALKING_G(service_instance)) == 0) {
        char *instance = sky_core_service_instance_id();
        char *service_instance = (char *) pemalloc(strlen(instance), 1);
        memcpy(service_instance, instance, strlen(instance));
        SKYWALKING_G(real_service_instance) = service_instance;
    }

    // todo if on
    if (1 == 1) {
        sky_plugin_redis_hooks();
    }

    // todo if on
    if (1 == 1) {
        sky_plugin_curl_hooks();
    }

    SKYWALKING_G(segments) = pemalloc(sizeof(HashTable), 1);
    zend_hash_init(SKYWALKING_G(segments), 0, NULL, delete_segments, 1);

    if (sky_core_report_ipc_init(SKYWALKING_G(mq_max_message_length))) {
        // register
        pthread_t thread_id;
        if (!pthread_create(&thread_id, 0, thread_sky_core_report_new, NULL)) {
            pthread_detach(thread_id);
        }
    }
    return 0;
}

static ZEND_RSRC_DTOR_FUNC(skywalking_dtor) {

};

void sky_core_module_free() {
    pefree(SKYWALKING_G(segments), 1);
}

void sky_core_request_init(zval *request, u_int64_t request_id) {
    if (strncmp(sapi_module.name, "fpm-fcgi", sizeof("fpm-fcgi") - 1) != 0) {
        return;
    }

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
        if (strcasecmp(SKYWALKING_G(oap_cross_process_protocol), "3.0") == 0) {
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

    zend_hash_index_add_ptr(SKYWALKING_G(segments), request_id, core_segment);
}

void sky_core_request_free(zval *response, u_int64_t request_id) {
    if (strncmp(sapi_module.name, "fpm-fcgi", sizeof("fpm-fcgi") - 1) != 0) {
        return;
    }
    zval_dtor(&SKYWALKING_G(curl_cache));

    if (SKYWALKING_G(segments) == NULL) {
        return;
    }

    sky_core_segment_t *core_segment = (sky_core_segment_t *) zend_hash_index_find_ptr(SKYWALKING_G(segments),
                                                                                       request_id);

    if (response == NULL) {
//        segment->setStatusCode(SG(sapi_headers).http_response_code);
    }
    sky_core_span_set_end_time(core_segment->spans[0]);


//    zend_string *persistent_id = zend_string_init(skywalking_persistent_id, strlen(skywalking_persistent_id), 0);
//    zend_resource *le = zend_hash_find_ptr(&EG(persistent_list), persistent_id);
//    zend_string_release(persistent_id);
//    sky_core_report_t *report = (sky_core_report_t *) le->ptr;
    sky_core_segment_set_service(core_segment, SKYWALKING_G(service));
    sky_core_segment_set_service_instance(core_segment, SKYWALKING_G(real_service_instance));

    char *json = NULL;
    sky_core_segment_to_json(&json, core_segment);
    efree(json);
    sky_core_report_ipc_send(json, strlen(json));
//    sky_core_report_push(report, json);

    zend_hash_index_del(SKYWALKING_G(segments), request_id);
//    efree(segment);
}