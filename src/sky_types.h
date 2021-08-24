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

#ifndef SKYAPM_SKY_APM_PHP_SDK_SKY_TYPES_H
#define SKYAPM_SKY_APM_PHP_SDK_SKY_TYPES_H

#endif //SKYAPM_SKY_APM_PHP_SDK_SKY_TYPES_H
#include <iostream>

typedef struct _json_rpc_ext {
    std::string traceid;
    std::string spanid;
    std::string parentid;
    std::string uri;
    std::string requestTime;
    std::string serviceName;
    std::string ServiceInstance;
    std::string endpoint;
    std::string address;
} json_rpc_ext;

typedef struct _swoft_json_rpc {
    std::string jsonrpc;
    std::string method;
    json_rpc_ext ext;
} swoft_json_rpc;

