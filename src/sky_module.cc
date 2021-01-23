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

#include <map>
#include <iostream>
#include "sky_module.h"
#include <boost/interprocess/ipc/message_queue.hpp>

#include "segment.h"
#include "sky_utils.h"
#include "sky_plugin_curl.h"
#include "sky_execute.h"
#include "manager.h"
#include "sky_shm.h"

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

    ManagerOptions opt;
    opt.version = SKYWALKING_G(version);
    opt.code = SKYWALKING_G(app_code);
    opt.grpc = SKYWALKING_G(grpc);
    opt.grpc_tls = SKYWALKING_G(grpc_tls_enable);
    opt.root_certs = SKYWALKING_G(grpc_tls_pem_root_certs);
    opt.private_key = SKYWALKING_G(grpc_tls_pem_private_key);
    opt.cert_chain = SKYWALKING_G(grpc_tls_pem_cert_chain);

    try {
        boost::interprocess::message_queue::remove("skywalking_queue");
        boost::interprocess::message_queue(boost::interprocess::create_only, "skywalking_queue", 1024, 20480);
        std::cout << "create" << std::endl;
    } catch(boost::interprocess::interprocess_exception &ex) {
        std::cout << "create" << ex.what() << std::endl;
    }

    new Manager(opt, s_info);
}

void sky_request_init(zval *request, uint64_t request_id) {
    array_init(&SKYWALKING_G(curl_header));

    zval *carrier = nullptr;
    zval *sw;
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
        peer = Z_STRVAL_P(zend_hash_str_find(Z_ARRVAL_P(swoole_header), "host", sizeof("host") - 1));

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

    std::map<uint64_t, Segment*> *segments;
    if (SKYWALKING_G(segment) == nullptr) {
        segments = new std::map<uint64_t, Segment*>;
        SKYWALKING_G(segment) = segments;
    } else {
        segments = static_cast<std::map<uint64_t, Segment *> *>SKYWALKING_G(segment);
    }

    auto *segment = new Segment(s_info->service, s_info->service_instance, SKYWALKING_G(version), header);
    auto const result = segments->insert(std::pair<uint64_t, Segment*>(request_id, segment));
    if (not result.second) {
        result.first->second = segment;
    }

    auto *span = segments->at(request_id)->createSpan(SkySpanType::Entry, SkySpanLayer::Http, 8001);
    span->setOperationName(uri);
    span->setPeer(peer);
    span->addTag("url", uri);
    segments->at(request_id)->createRefs();

}



void sky_request_flush(zval *response, uint64_t request_id) {
    auto *segment = sky_get_segment(nullptr, request_id);

    if (response == nullptr) {
        segment->setStatusCode(SG(sapi_headers).http_response_code);
    }
    std::string msg = segment->marshal();
    delete segment;

    try {
        boost::interprocess::message_queue mq(boost::interprocess::open_or_create, "skywalking_queue", 1024, 20480);
        std::cout << msg.length() << std::endl;
        mq.send(msg.data(), msg.size(), 0);
    } catch(boost::interprocess::interprocess_exception &ex) {
        std::cout << "send" << ex.what() << std::endl;
    }
}