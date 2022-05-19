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


#include "sky_utils.h"
#include "php_skywalking.h"
#include "zend_variables.h"

//// protect SKYWALKING_G(segment)'s insert and remove
//static std::mutex segments_mutex;
//
//bool starts_with(const char *pre, const char *str) {
//    size_t len_pre = strlen(pre),
//            len_str = strlen(str);
//    return len_str < len_pre ? false : memcmp(pre, str, len_pre) == 0;
//}
//
//std::string get_page_request_uri() {
//    zval *carrier;
//    zval *request_uri;
//
//    std::string uri;
//
//    if (strcasecmp("cli", sapi_module.name) == 0) {
//        uri = "cli";
//    } else {
//        zend_bool jit_initialization = PG(auto_globals_jit);
//
//        if (jit_initialization) {
//            zend_string *server_str = zend_string_init("_SERVER", sizeof("_SERVER") - 1, 0);
//            zend_is_auto_global(server_str);
//            zend_string_release(server_str);
//        }
//        carrier = zend_hash_str_find(&EG(symbol_table), ZEND_STRL("_SERVER"));
//
//        request_uri = zend_hash_str_find(Z_ARRVAL_P(carrier), "REQUEST_URI", sizeof("REQUEST_URI") - 1);
//        uri = Z_STRVAL_P(request_uri);
//    }
//    return uri;
//}
//
//std::string get_page_request_peer() {
//    zval *carrier;
//    zval *request_host;
//    zval *request_port;
//
//    std::string peer;
//
//    zend_bool jit_initialization = PG(auto_globals_jit);
//
//    if (jit_initialization) {
//        zend_string *server_str = zend_string_init("_SERVER", sizeof("_SERVER") - 1, 0);
//        zend_is_auto_global(server_str);
//        zend_string_release(server_str);
//    }
//    carrier = zend_hash_str_find(&EG(symbol_table), ZEND_STRL("_SERVER"));
//
//    request_host = zend_hash_str_find(Z_ARRVAL_P(carrier), "HTTP_HOST", sizeof("HTTP_HOST") - 1);
//    request_port = zend_hash_str_find(Z_ARRVAL_P(carrier), "SERVER_PORT", sizeof("SERVER_PORT") - 1);
//    if (request_host == nullptr) {
//        request_host = zend_hash_str_find(Z_ARRVAL_P(carrier), "SERVER_ADDR", sizeof("SERVER_ADDR") - 1);
//    }
//
//    if (request_host != nullptr && request_port != nullptr) {
//        peer = std::string(Z_STRVAL_P(request_host)) + ":" + Z_STRVAL_P(request_port);
//    }
//
//    return peer;
//}
//
//zval *sky_read_property(zval *obj, const char *property, int parent) {
//    if (Z_TYPE_P(obj) == IS_OBJECT) {
//        zend_object *object = obj->value.obj;
//        zend_class_entry *ce;
//
//        if (parent == 0) {
//            ce = object->ce;
//        } else {
//            ce = object->ce->parent;
//        }
//
//#if PHP_VERSION_ID < 80000
//        return zend_read_property(ce, obj, property, strlen(property), 0, nullptr);
//#else
//        return zend_read_property(ce, object, property, strlen(property), 0, nullptr);
//#endif
//    }
//    return nullptr;
//}
//
//int64_t sky_find_swoole_fd(zend_execute_data *execute_data) {
//
//    if (execute_data->prev_execute_data) {
//        uint32_t arg_count = ZEND_CALL_NUM_ARGS(execute_data->prev_execute_data);
//        if (arg_count == 2) {
//            zval *sw_request = ZEND_CALL_ARG(execute_data->prev_execute_data, 1);
//            if (Z_TYPE_P(sw_request) == IS_OBJECT) {
//                if (strcmp(ZSTR_VAL(Z_OBJ_P(sw_request)->ce->name), "Swoole\\Http\\Request") == 0) {
//                    zval *fd = sky_read_property(sw_request, "fd", 0);
//                    return Z_LVAL_P(fd);
//                }
//            }
//        }
//        return sky_find_swoole_fd(execute_data->prev_execute_data);
//    }
//
//    return -1;
//}
//
//SkyCoreSegment *sky_get_segment(int64_t request_id) {
//    if (SKYWALKING_G(segment) == nullptr) {
//        return nullptr;
//    }
//    auto *segments = static_cast<std::unordered_map<uint64_t, SkyCoreSegment *> *>SKYWALKING_G(segment);
//    auto it = segments->find(request_id);
//        if (it != segments->end()) {
//        return it->second;
//    }
//    return nullptr;
//}
//
sky_core_segment_t *sky_util_find_segment_idx(zend_execute_data *execute_data, int64_t request_id) {

    if (SKYWALKING_G(segments) == NULL) {
        // can't be here, since module_init has assigned a map to SKYWALKING_G(segment)
        return NULL;
    }

//    auto *segments = static_cast<std::unordered_map<uint64_t, SkyCoreSegment *> *>SKYWALKING_G(segment);
    bool do_search = false;
    zend_ulong key = 0;
    if (request_id >= 0) {
        key = request_id;
        do_search = true;
    } else {
        if (SKYWALKING_G(is_swoole)) {
//            int64_t fd = sky_find_swoole_fd(execute_data);
//            if (fd > 0) {
//                key = fd;
//                do_search = true;
//            }
        } else {
            do_search = true;
        }
    }

    if (do_search) {
        zval *segment = zend_hash_index_find(SKYWALKING_G(segments), key);
        if (segment) {
            sky_core_segment_t *core_segment = (sky_core_segment_t *) Z_PTR_P(segment);
            return core_segment;
        }
    }

    return NULL;
}

bool starts_with(const char *pre, const char *str) {
    size_t pre_len = strlen(pre);
    size_t str_len = strlen(str);
    return str_len < pre_len ? false : memcmp(pre, str, pre_len) == 0;
}

int sky_util_call_user_func(const char *name, zval *retval_ptr, uint32_t count, zval params[]) {
    zval func;
    ZVAL_STRING(&func, name);
    int ret = call_user_function(CG(function_table), NULL, &func, retval_ptr, count, params);
    zval_dtor(&func);

    for (int i = 0; i < count; i++) {
        zval_dtor(&params[i]);
    }
    return ret;
}

//bool sky_insert_segment(uint64_t request_id, SkyCoreSegment *segment) {
//    auto *segments = static_cast<std::unordered_map<uint64_t, SkyCoreSegment *> *>SKYWALKING_G(segment);
//    std::lock_guard<std::mutex> lock(segments_mutex);
//
//    std::pair<std::unordered_map<uint64_t, SkyCoreSegment *>::iterator, bool> result;
//    result = segments->insert(std::pair<uint64_t, SkyCoreSegment *>(request_id, segment));
//
//    return result.second;
//}
//
//void sky_remove_segment(uint64_t request_id) {
//    auto *segments = static_cast<std::unordered_map<uint64_t, SkyCoreSegment *> *>SKYWALKING_G(segment);
//    std::lock_guard<std::mutex> lock(segments_mutex);
//
//    segments->erase(request_id);
//}
//
//std::string sky_get_class_name(zval *obj) {
//    if (Z_TYPE_P(obj) == IS_OBJECT) {
//        zend_object *object = obj->value.obj;
//        return ZSTR_VAL(object->ce->name);
//    }
//    return "";
//}
//
//long getUnixTimeStamp() {
//    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
//            std::chrono::system_clock::now().time_since_epoch());
//    return ms.count();
//}
//
//std::string sky_json_encode(zval *parameter) {
//  std::string str;
//  smart_str buf = {nullptr};
//  zend_long options = 256;
//#if PHP_VERSION_ID >= 70100
//  if (php_json_encode(&buf, parameter, (int) options) != SUCCESS) {
//        smart_str_free(&buf);
//        return str;
//    }
//#else
//  php_json_encode(&buf, parameter, (int) options);
//#endif
//  smart_str_0(&buf);
//  if (buf.s != nullptr) {
//    str = std::string(ZSTR_VAL(buf.s));
//    smart_str_free(&buf);
//  }
//  return str;
//}
