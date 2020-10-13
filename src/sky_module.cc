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

#include "sky_module.h"

#include "segment.h"
#include "sky_utils.h"
#include "sky_curl.h"
#include "sky_execute.h"
#include "sys/mman.h"
#include "manager.h"

extern struct service_info *s_info;
extern int fd[2];

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

    if (pipe(fd) == 0) {
        int protection = PROT_READ | PROT_WRITE;
        int visibility = MAP_SHARED | MAP_ANONYMOUS;

        s_info = (struct service_info *) mmap(nullptr, sizeof(struct service_info), protection, visibility, -1, 0);

        ManagerOptions opt;
        opt.version = SKYWALKING_G(version);
        opt.code = SKYWALKING_G(app_code);
        opt.grpc = SKYWALKING_G(grpc);
        opt.grpc_tls = SKYWALKING_G(grpc_tls_enable);
        opt.root_certs = SKYWALKING_G(grpc_tls_pem_root_certs);
        opt.private_key = SKYWALKING_G(grpc_tls_pem_private_key);
        opt.cert_chain = SKYWALKING_G(grpc_tls_pem_cert_chain);

        new Manager(opt, s_info, fd);
    }
}

void sky_request_init() {
    array_init(&SKYWALKING_G(curl_header));

    zval *carrier = nullptr;
    zval *sw;

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

    std::string header(sw != nullptr ? Z_STRVAL_P(sw) : "");
    auto *segment = new Segment(s_info->service, s_info->service_instance, SKYWALKING_G(version), header);
    SKYWALKING_G(segment) = segment;

    auto *span = segment->createSpan(SkySpanType::Entry, SkySpanLayer::Http, 8001);


    // init entry span
    auto uri = get_page_request_uri();
    auto peer = get_page_request_peer();
    span->setOperationName(uri);
    span->setPeer(peer);
    span->addTag("url", uri);
    segment->createRefs();

}

void sky_request_flush() {
    auto *segment = static_cast<Segment *>(SKYWALKING_G(segment));
    std::string msg = segment->marshal(SG(sapi_headers).http_response_code);
    delete segment;
    write(fd[1], msg.c_str(), msg.length());
}