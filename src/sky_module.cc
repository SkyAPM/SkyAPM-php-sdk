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


#include <unordered_map>
#include <iostream>
#include "sky_module.h"
#include <boost/interprocess/ipc/message_queue.hpp>
#include <fstream>

#include "segment.h"
#include "sky_utils.h"
#include "sky_plugin_curl.h"
#include "sky_execute.h"
#include "manager.h"
#include "sky_plugin_error.h"
#include "sky_log.h"
#include "sky_rate_limit.h"

extern struct service_info *s_info;

extern void (*ori_execute_ex)(zend_execute_data *execute_data);

extern void (*ori_execute_internal)(zend_execute_data *execute_data, zval *return_value);

extern void (*orig_curl_exec)(INTERNAL_FUNCTION_PARAMETERS);

extern void (*orig_curl_setopt)(INTERNAL_FUNCTION_PARAMETERS);

extern void (*orig_curl_setopt_array)(INTERNAL_FUNCTION_PARAMETERS);

extern void (*orig_curl_close)(INTERNAL_FUNCTION_PARAMETERS);

void sky_module_init() {
    ori_execute_ex = zend_execute_ex;
    zend_execute_ex = sky_execute_ex;

    ori_execute_internal = zend_execute_internal;
    zend_execute_internal = sky_execute_internal;

    if (SKYWALKING_G(error_handler_enable)) {
        sky_plugin_error_init();
    }

    // bind curl
    zend_function *old_function;
    if ((old_function = SKY_OLD_FN("curl_exec")) != nullptr) {
        orig_curl_exec = old_function->internal_function.handler;
        old_function->internal_function.handler = sky_curl_exec_handler;
    }
    if ((old_function = SKY_OLD_FN("curl_setopt")) != nullptr) {
        orig_curl_setopt = old_function->internal_function.handler;
        old_function->internal_function.handler = sky_curl_setopt_handler;
    }
    if ((old_function = SKY_OLD_FN("curl_setopt_array")) != nullptr) {
        orig_curl_setopt_array = old_function->internal_function.handler;
        old_function->internal_function.handler = sky_curl_setopt_array_handler;
    }
    if ((old_function = SKY_OLD_FN("curl_close")) != nullptr) {
        orig_curl_close = old_function->internal_function.handler;
        old_function->internal_function.handler = sky_curl_close_handler;
    }

    std::unordered_map<uint64_t, Segment *> *segments = new std::unordered_map<uint64_t, Segment *>;
    SKYWALKING_G(segment) = segments;

    FixedWindowRateLimiter *rate_limiter = new FixedWindowRateLimiter(SKYWALKING_G(sample_n_per_3_secs));
    SKYWALKING_G(rate_limiter) = rate_limiter;

    if (SKYWALKING_G(mq_unique)) {
        sprintf(s_info->mq_name, "skywalking_queue");
    }else{
        sprintf(s_info->mq_name, "skywalking_queue_%d", getpid());
    }

    try {
        boost::interprocess::message_queue::remove(s_info->mq_name);
        boost::interprocess::message_queue(
                boost::interprocess::open_or_create,
                s_info->mq_name,
                1024,
                SKYWALKING_G(mq_max_message_length),
                boost::interprocess::permissions(0666)
        );
    } catch (boost::interprocess::interprocess_exception &ex) {
        php_error(E_WARNING, "%s %s", "[skywalking] create queue fail ", ex.what());
    }

    ManagerOptions opt;
    opt.version = SKYWALKING_G(version);
    opt.code = SKYWALKING_G(app_code);
    opt.grpc = SKYWALKING_G(grpc);
    opt.grpc_tls = SKYWALKING_G(grpc_tls_enable);
    opt.root_certs = SKYWALKING_G(grpc_tls_pem_root_certs);
    opt.private_key = SKYWALKING_G(grpc_tls_pem_private_key);
    opt.cert_chain = SKYWALKING_G(grpc_tls_pem_cert_chain);
    opt.authentication = SKYWALKING_G(authentication);
    opt.instance_name = SKYWALKING_G(instance_name);

    Manager::init(opt, s_info);
}

void sky_module_cleanup() {
    char mq_name[32];

    sprintf(mq_name, "skywalking_queue_%d", getpid());
    if (SKYWALKING_G(mq_unique) || strcmp(s_info->mq_name, mq_name) == 0) {
        boost::interprocess::message_queue::remove(s_info->mq_name);
    }

    std::unordered_map<uint64_t, Segment *> *segments = static_cast<std::unordered_map<uint64_t, Segment *> *>(SKYWALKING_G(segment));
    for (auto entry : *segments) {
        delete entry.second;
    }

    delete segments;
    delete static_cast<FixedWindowRateLimiter*>(SKYWALKING_G(rate_limiter));
}

void sky_request_init(zval *request, uint64_t request_id) {
    array_init(&SKYWALKING_G(curl_header));

    if (!static_cast<FixedWindowRateLimiter*>(SKYWALKING_G(rate_limiter))->validate()) {
        auto *segment = new Segment(s_info->service, s_info->service_instance, SKYWALKING_G(version), "");
        segment->setSkip(true);
        (void)sky_insert_segment(request_id, segment);

        return;
    }

    zval *carrier = nullptr;
    zval *sw, *peer_val;
    std::string header;
    std::string uri;
    std::string peer;

    if (request != nullptr) {
        zval *swoole_header = sky_read_property(request, "header", 0);
        zval *swoole_server = sky_read_property(request, "server", 0);

        if (SKYWALKING_G(version) == 8) {
            sw = zend_hash_str_find(Z_ARRVAL_P(swoole_header), "sw8", sizeof("sw8") - 1);
        } else {
            sw = nullptr;
        }

        header = (sw != nullptr ? Z_STRVAL_P(sw) : "");

        uri = Z_STRVAL_P(zend_hash_str_find(Z_ARRVAL_P(swoole_server), "request_uri", sizeof("request_uri") - 1));

        peer_val = zend_hash_str_find(Z_ARRVAL_P(swoole_header), "host", sizeof("host") - 1);
        if (peer_val != nullptr) {
            peer = Z_STRVAL_P(peer_val);
        } else {
            char hostname[HOST_NAME_MAX + 1];
            if (gethostname(hostname, sizeof(hostname))) {
                hostname[0] = '\0';
            }
            peer_val = zend_hash_str_find(Z_ARRVAL_P(swoole_server), "server_port", sizeof("server_port") - 1);
            peer += hostname;
            peer += ":";
            peer += std::to_string(Z_LVAL_P(peer_val));
        }
    } else {
        zend_bool jit_initialization = PG(auto_globals_jit);

        if (jit_initialization) {
            zend_string *server_str = zend_string_init("_SERVER", sizeof("_SERVER") - 1, 0);
            zend_is_auto_global(server_str);
            zend_string_release(server_str);
        }
        carrier = zend_hash_str_find(&EG(symbol_table), ZEND_STRL("_SERVER"));

        if (SKYWALKING_G(version) == 5) {
            sw = zend_hash_str_find(Z_ARRVAL_P(carrier), "HTTP_SW3", sizeof("HTTP_SW3") - 1);
        } else if (SKYWALKING_G(version) == 6 || SKYWALKING_G(version) == 7) {
            sw = zend_hash_str_find(Z_ARRVAL_P(carrier), "HTTP_SW6", sizeof("HTTP_SW6") - 1);
        } else if (SKYWALKING_G(version) == 8) {
            sw = zend_hash_str_find(Z_ARRVAL_P(carrier), "HTTP_SW8", sizeof("HTTP_SW8") - 1);
        } else {
            sw = nullptr;
        }

        header = (sw != nullptr ? Z_STRVAL_P(sw) : "");
        uri = get_page_request_uri();
        peer = get_page_request_peer();
    }

    std::unordered_map<uint64_t, Segment *> *segments = static_cast<std::unordered_map<uint64_t, Segment *> *>SKYWALKING_G(segment);

    auto *segment = new Segment(s_info->service, s_info->service_instance, SKYWALKING_G(version), header);
    (void)sky_insert_segment(request_id, segment);

    auto *span = segments->at(request_id)->createSpan(SkySpanType::Entry, SkySpanLayer::Http, 8001);
    span->setOperationName(uri);
    span->setPeer(peer);
    span->addTag("url", uri);
    segments->at(request_id)->createRefs();

    zval *request_method = zend_hash_str_find(Z_ARRVAL(PG(http_globals)[TRACK_VARS_SERVER]), ZEND_STRL("REQUEST_METHOD"));
    if (request_method != NULL) {
        span->addTag("http.method", Z_STRVAL_P(request_method));
    }
}


void sky_request_flush(zval *response, uint64_t request_id) {
    auto *segment = sky_get_segment(nullptr, request_id);
    if (nullptr == segment) {
        return;
    }
    if (segment->skip()) {
        delete segment;
        sky_remove_segment(request_id);

        return;
    }

    if (response == nullptr) {
        segment->setStatusCode(SG(sapi_headers).http_response_code);
    }

    std::string msg = segment->marshal();
    delete segment;
    sky_remove_segment(request_id);

    int msg_length = static_cast<int>(msg.size());
    int max_length = SKYWALKING_G(mq_max_message_length);
    if (msg_length > max_length) {
        sky_log("message is too big: " + std::to_string(msg_length) + ", mq_max_message_length=" + std::to_string(max_length));
        return;
    }

    try {
        boost::interprocess::message_queue mq(
                boost::interprocess::open_only,
                s_info->mq_name
        );
        if (!mq.try_send(msg.data(), msg.size(), 0)) {
            sky_log("sky_request_flush message_queue is fulled");
        }
    } catch (boost::interprocess::interprocess_exception &ex) {
        sky_log("sky_request_flush message_queue ex" + std::string(ex.what()));
        php_error(E_WARNING, "%s %s", "[skywalking] open queue fail ", ex.what());
    }
}
