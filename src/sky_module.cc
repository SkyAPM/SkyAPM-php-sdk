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
#include <fstream>

#include "sky_core_segment.h"
#include "sky_utils.h"
#include "sky_plugin_curl.h"
#include "sky_execute.h"
#include "sky_plugin_error.h"
#include "sky_log.h"
#include "sky_rate_limit.h"
#include "sky_go_wrapper.h"
#include "sky_go_utils.h"
#include "sys/ipc.h"
#include "sys/shm.h"

extern mutex_service *ms;

extern void (*ori_execute_ex)(zend_execute_data *execute_data);

extern void (*ori_execute_internal)(zend_execute_data *execute_data, zval *return_value);

extern void (*orig_curl_exec)(INTERNAL_FUNCTION_PARAMETERS);

extern void (*orig_curl_setopt)(INTERNAL_FUNCTION_PARAMETERS);

extern void (*orig_curl_setopt_array)(INTERNAL_FUNCTION_PARAMETERS);

extern void (*orig_curl_close)(INTERNAL_FUNCTION_PARAMETERS);

void sky_module_init() {

    key_t key = ftok("./", 1);
    if (key < 0) {
        return;
    }
    const int id = shmget(key, sizeof(mutex_service), IPC_CREAT | 0666);
    if (id < 0) {
        return;
    }
    ms = (mutex_service *) shmat(id, nullptr, SHM_R | SHM_W);
    ms->id = id;
    pthread_mutexattr_init(&(ms->lock_attr));
    pthread_mutexattr_setpshared(&(ms->lock_attr), PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&(ms->lock), &(ms->lock_attr));



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

    auto *segments = new std::unordered_map<uint64_t, SkyCoreSegment *>;
    SKYWALKING_G(segment) = segments;

    auto *rate_limiter = new FixedWindowRateLimiter(SKYWALKING_G(sample_n_per_3_secs));
    SKYWALKING_G(rate_limiter) = rate_limiter;

    GoString address = NewGoString(SKYWALKING_G(grpc), sizeof(SKYWALKING_G(grpc)) - 1);
    GoString server = NewGoString(SKYWALKING_G(app_code), sizeof(SKYWALKING_G(app_code)) - 1);
    GoString instance = NewGoString(SKYWALKING_G(instance_name), sizeof(SKYWALKING_G(instance_name)) - 1);
    GoString realInstance = NewProtocol(address, server, instance);
    pthread_mutex_lock(&(ms->lock));
    strcpy(ms->service, server.p);
    strcpy(ms->serviceInstance, std::string(realInstance.p, realInstance.n).c_str());
    pthread_mutex_unlock(&(ms->lock));
    ReportInstanceProperties();
}

void sky_module_cleanup() {

    if (ms != nullptr) {
        pthread_mutex_destroy(&(ms->lock));
        pthread_mutexattr_destroy(&(ms->lock_attr));
        shmctl(ms->id, IPC_RMID, nullptr);
    }

    auto *segments = static_cast<std::unordered_map<uint64_t, SkyCoreSegment *> *>(SKYWALKING_G(segment));
    for (auto entry : *segments) {
        delete entry.second;
    }

    delete segments;
    delete static_cast<FixedWindowRateLimiter*>(SKYWALKING_G(rate_limiter));
}

void sky_request_init(zval *request, uint64_t request_id) {
    array_init(&SKYWALKING_G(curl_header));

    if (!static_cast<FixedWindowRateLimiter*>(SKYWALKING_G(rate_limiter))->validate()) {
        auto *segment = new SkyCoreSegment("");
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

    }

}
