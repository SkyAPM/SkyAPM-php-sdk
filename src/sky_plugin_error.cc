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

#include "sky_plugin_error.h"
#include "php_skywalking.h"
#include "segment.h"
#include <string>

#if PHP_VERSION_ID < 80000
extern void (*orig_error_cb)(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args);

void (*orig_error_cb)(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args) = nullptr;

void sky_plugin_error_cb(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args) {
#else
extern void (*orig_error_cb)(int type, const char *error_filename, const uint32_t error_lineno, zend_string *message);

void (*orig_error_cb)(int type, const char *error_filename, const uint32_t error_lineno, zend_string *message) = nullptr;

void sky_plugin_error_cb(int type, const char *error_filename, const uint32_t error_lineno, zend_string *message) {
#endif
    std::string level;
    bool isError = EG(error_reporting) & type;
    switch (type) {
        case E_ERROR:
        case E_PARSE:
        case E_CORE_ERROR:
        case E_COMPILE_ERROR:
        case E_USER_ERROR:
        case E_RECOVERABLE_ERROR:
            level = "ERROR";
            break;
        case E_WARNING:
        case E_CORE_WARNING:
        case E_COMPILE_WARNING:
        case E_USER_WARNING:
            level = "WARNING";
            break;
        case E_NOTICE:
        case E_USER_NOTICE:
        case E_STRICT:
        case E_DEPRECATED:
        case E_USER_DEPRECATED:	
            level = "NOTICE";
            break;
        default:
            level = "E_" + std::to_string(type);		
            break;
    }

    std::string log = error_filename;
#if PHP_VERSION_ID < 80000
    char *msg;
    va_list args_copy;
    va_copy(args_copy, args);
    vspprintf(&msg, 0, format, args_copy);
    va_end(args_copy);
    log += "(" + std::to_string(error_lineno) + "): " + msg;
    efree(msg);
#else
    log +=  "(" + std::to_string(error_lineno) + "): " + message->val;
#endif

    if (!SKYWALKING_G(is_swoole) && SKYWALKING_G(enable) && SKYWALKING_G(segment) != nullptr) {
        auto *segments = static_cast<std::map<uint64_t, Segment *> *>SKYWALKING_G(segment);
        auto segment = segments->at(0);
        auto span = segment->firstSpan();
        span->addLog(level, log);
        if (isError) {
            span->setIsError(true);
        }
    }

#if PHP_VERSION_ID < 80000
    orig_error_cb(type, error_filename, error_lineno, format, args);
#else
    orig_error_cb(type, error_filename, error_lineno, message);
#endif
}

void sky_plugin_error_init() {
    orig_error_cb = zend_error_cb;
    zend_error_cb = sky_plugin_error_cb;
}
