// Licensed to the Apache Software Foundation (ASF) under one or more
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

#ifndef SKYWALKING_MANAGER_H
#define SKYWALKING_MANAGER_H

#include <string>
#include <vector>
#include "grpc/grpc.h"
#include "grpc++/grpc++.h"

#if (defined(unix) || defined(__unix__) || defined(__unix)) && !defined(__APPLE__)
#define PLATFORM_NAME "Unix"
#elif defined(__linux__)
#define PLATFORM_NAME "Linux"
#elif defined(__APPLE__) && defined(__MACH__)
#define PLATFORM_NAME "MacOS"
#elif defined(__FreeBSD__)
#define PLATFORM_NAME "FreeBSD"
#else
#define PLATFORM_NAME ""
#endif

struct ManagerOptions {
    int version;
    std::string code;
    std::string grpc;
    bool grpc_tls;
    std::string root_certs;
    std::string private_key;
    std::string cert_chain;
    std::string authentication;
};

class Manager {

public:
    Manager(const ManagerOptions &options, struct service_info *info);

    static std::string generateUUID();

private:

    static void login(const ManagerOptions &options, struct service_info *info);

    [[noreturn]] static void heartbeat(const ManagerOptions &options, const std::string &serviceInstance);

    [[noreturn]] static void consumer(const ManagerOptions &options);

    static void logger(const std::string &log);

    static std::vector<std::string> getIps();

    static std::shared_ptr<grpc::ChannelCredentials> getCredentials(const ManagerOptions &options);
};


#endif //SKYWALKING_MANAGER_H
