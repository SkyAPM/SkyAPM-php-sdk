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

#include "sky_utils.h"

#include "php.h"
#include "zend_smart_str.h"
#include "main/SAPI.h"
#include "stdbool.h"
#include "string.h"

bool starts_with(const char *pre, const char *str) {
    size_t len_pre = strlen(pre),
            len_str = strlen(str);
    return len_str < len_pre ? false : memcmp(pre, str, len_pre) == 0;
}

char *get_page_request_uri() {
    zval *carrier = NULL;
    zval *request_uri;

    smart_str uri = {0};

    if (strcasecmp("cli", sapi_module.name) == 0) {
        smart_str_appendl(&uri, "cli", strlen("cli"));
    } else {
        zend_bool jit_initialization = PG(auto_globals_jit);

        if (jit_initialization) {
            zend_string *server_str = zend_string_init("_SERVER", sizeof("_SERVER") - 1, 0);
            zend_is_auto_global(server_str);
            zend_string_release(server_str);
        }
        carrier = zend_hash_str_find(&EG(symbol_table), ZEND_STRL("_SERVER"));

        request_uri = zend_hash_str_find(Z_ARRVAL_P(carrier), "REQUEST_URI", sizeof("REQUEST_URI") - 1);
        smart_str_appendl(&uri, Z_STRVAL_P(request_uri), strlen(Z_STRVAL_P(request_uri)));
    }

    smart_str_0(&uri);
    if (uri.s != NULL) {
        char *uris = emalloc(strlen(ZSTR_VAL(uri.s)) + 1);
        strcpy(uris, ZSTR_VAL(uri.s));
        smart_str_free(&uri);
        return uris;
    }
    return NULL;
}

char *get_page_request_peer() {
    zval *carrier = NULL;
    zval *request_host = NULL;
    zval *request_port = NULL;

    char *peer = NULL;
    size_t peer_l = 0;

    zend_bool jit_initialization = PG(auto_globals_jit);

    if (jit_initialization) {
        zend_string *server_str = zend_string_init("_SERVER", sizeof("_SERVER") - 1, 0);
        zend_is_auto_global(server_str);
        zend_string_release(server_str);
    }
    carrier = zend_hash_str_find(&EG(symbol_table), ZEND_STRL("_SERVER"));

    request_host = zend_hash_str_find(Z_ARRVAL_P(carrier), "HTTP_HOST", sizeof("HTTP_HOST") - 1);
    request_port = zend_hash_str_find(Z_ARRVAL_P(carrier), "SERVER_PORT", sizeof("SERVER_PORT") - 1);
    if (request_host == NULL) {
        request_host = zend_hash_str_find(Z_ARRVAL_P(carrier), "SERVER_ADDR", sizeof("SERVER_ADDR") - 1);
    }

    if (request_host != NULL && request_port != NULL) {
        peer_l = snprintf(NULL, 0, "%s:%s", Z_STRVAL_P(request_host), Z_STRVAL_P(request_port));
        peer = emalloc(peer_l + 1);
        snprintf(peer, peer_l + 1, "%s:%s", Z_STRVAL_P(request_host), Z_STRVAL_P(request_port));
    }

    return peer;
}