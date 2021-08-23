//
// Created by mick on 2021/8/19.
//

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

