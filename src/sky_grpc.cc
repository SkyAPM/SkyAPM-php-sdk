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


#include "sky_grpc.h"
#include "segment.h"

#include "php_skywalking.h"
#include "sky_utils.h"

Span *sky_grpc(zend_execute_data *execute_data, char *class_name, char *function_name) {
    std::string _class_name(class_name);
    std::string _function_name(function_name);

    if (_function_name == "_simpleRequest" || _function_name == "_clientStreamRequest" ||
        _function_name == "_serverStreamRequest" || _function_name == "_bidiRequest") {

        auto *segment = static_cast<Segment *>(SKYWALKING_G(segment));
        auto *span = segment->createSpan(SkySpanType::Exit, SkySpanLayer::RPCFramework, 23);
        span->setOperationName(_class_name + "->" + _function_name);
        span->addTag("rpc.type", "grpc");

        zval *method = ZEND_CALL_ARG(execute_data, 1);
        if (Z_TYPE_P(method) == IS_STRING) {
            span->addTag("rpc.method", Z_STRVAL_P(method));
        }

        zval *hostname = sky_read_property(&(execute_data->This), "hostname", 1);
        zval *hostname_override = sky_read_property(&(execute_data->This), "hostname_override", 1);

        if (hostname_override != nullptr && Z_TYPE_P(hostname_override) == IS_STRING) {
            span->setPeer(Z_STRVAL_P(hostname_override));
        } else if (hostname != nullptr && Z_TYPE_P(hostname) == IS_STRING) {
            span->setPeer(Z_STRVAL_P(hostname));
        }

        return span;
    }


    return nullptr;
}