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

#include "php_skywalking.h"

bool starts_with(const char *pre, const char *str) {
    size_t len_pre = strlen(pre),
            len_str = strlen(str);
    return len_str < len_pre ? false : memcmp(pre, str, len_pre) == 0;
}

std::string get_page_request_uri() {
    zval *carrier;
    zval *request_uri;

    std::string uri;

    if (strcasecmp("cli", sapi_module.name) == 0) {
        uri = "cli";
    } else {
        zend_bool jit_initialization = PG(auto_globals_jit);

        if (jit_initialization) {
            zend_string *server_str = zend_string_init("_SERVER", sizeof("_SERVER") - 1, 0);
            zend_is_auto_global(server_str);
            zend_string_release(server_str);
        }
        carrier = zend_hash_str_find(&EG(symbol_table), ZEND_STRL("_SERVER"));

        request_uri = zend_hash_str_find(Z_ARRVAL_P(carrier), "REQUEST_URI", sizeof("REQUEST_URI") - 1);
        uri = Z_STRVAL_P(request_uri);
    }
    return uri;
}

std::string get_page_request_peer() {
    zval *carrier;
    zval *request_host;
    zval *request_port;

    std::string peer;

    zend_bool jit_initialization = PG(auto_globals_jit);

    if (jit_initialization) {
        zend_string *server_str = zend_string_init("_SERVER", sizeof("_SERVER") - 1, 0);
        zend_is_auto_global(server_str);
        zend_string_release(server_str);
    }
    carrier = zend_hash_str_find(&EG(symbol_table), ZEND_STRL("_SERVER"));

    request_host = zend_hash_str_find(Z_ARRVAL_P(carrier), "HTTP_HOST", sizeof("HTTP_HOST") - 1);
    request_port = zend_hash_str_find(Z_ARRVAL_P(carrier), "SERVER_PORT", sizeof("SERVER_PORT") - 1);
    if (request_host == nullptr) {
        request_host = zend_hash_str_find(Z_ARRVAL_P(carrier), "SERVER_ADDR", sizeof("SERVER_ADDR") - 1);
    }

    if (request_host != nullptr && request_port != nullptr) {
        peer = std::string(Z_STRVAL_P(request_host)) + ":" + Z_STRVAL_P(request_port);
    }

    return peer;
}

zval *sky_read_property(zval *obj, const char *property, int parent) {
    if (Z_TYPE_P(obj) == IS_OBJECT) {
        zend_object *object = obj->value.obj;
        zend_class_entry *ce;

        if (parent == 0) {
            ce = object->ce;
        } else {
            ce = object->ce->parent;
        }

#if PHP_VERSION_ID < 80000
        return zend_read_property(ce, obj, property, strlen(property), 0, nullptr);
#else
        return zend_read_property(ce, object, property, strlen(property), 0, nullptr);
#endif
    }
    return nullptr;
}

std::string sky_get_class_name(zval *obj) {
    if (Z_TYPE_P(obj) == IS_OBJECT) {
        zend_object *object = obj->value.obj;
        return ZSTR_VAL(object->ce->name);
    }
    return "";
}
