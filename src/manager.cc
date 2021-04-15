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

#include "manager.h"
#include <thread>
#include <iostream>
#include <string>
#include <zconf.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <sstream>
#include <random>
#include <fstream>
#include <queue>
#include "management/Management.grpc.pb.h"
#include "language-agent/Tracing.grpc.pb.h"
#include "grpc/grpc.h"
#include "grpc++/grpc++.h"
#include "segment.h"
#include <google/protobuf/util/json_util.h>
#include "common.h"
#include "sky_shm.h"
#include <boost/interprocess/ipc/message_queue.hpp>

#include "php_skywalking.h"
#include "sky_log.h"

std::queue<std::string> messageQueue;
static pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t cond_mx = PTHREAD_MUTEX_INITIALIZER;

extern struct service_info *s_info;

Manager::Manager(const ManagerOptions &options, struct service_info *info) {


    std::thread th(login, options, info);
    th.detach();

    std::thread c(consumer, options);
    c.detach();

//    std::thread s(sender, options);
//    s.detach();

    sky_log("the apache skywalking php plugin mounted");
}

void Manager::login(const ManagerOptions &options, struct service_info *info) {

    std::shared_ptr<grpc::Channel> channel(grpc::CreateChannel(options.grpc, getCredentials(options)));
    std::unique_ptr<ManagementService::Stub> stub(ManagementService::NewStub(channel));

    bool status = false;

    while (!status) {
        grpc::ClientContext context;
        InstanceProperties properties;
        Commands commands;
        if (!options.authentication.empty()) {
            context.AddMetadata("authentication", options.authentication);
        }

        auto ips = getIps();

        std::string instance;
        if (!ips.empty()) {
            // todo port
            instance = generateUUID() + "@" + ips[0];
        }

        properties.set_service(options.code);
        properties.set_serviceinstance(instance);
        auto osName = properties.add_properties();
        osName->set_key("os_name");
        osName->set_value(PLATFORM_NAME);

        char name[256] = {0};
        gethostname(name, sizeof(name));
        auto hostName = properties.add_properties();
        hostName->set_key("host_name");
        hostName->set_value(name);

        std::ostringstream strPid;
        strPid << getpid();
        auto pid = properties.add_properties();
        pid->set_key("process_no");
        pid->set_value(strPid.str());

        auto language = properties.add_properties();
        language->set_key("language");
        language->set_value("php");

        for (const auto &ip:ips) {
            auto tmp = properties.add_properties();
            tmp->set_key("ipv4");
            tmp->set_value(ip);
        }
        std::string msg = properties.SerializeAsString();
        auto rc = stub->reportInstanceProperties(&context, properties, &commands);
        if (rc.ok() && info != nullptr) {
            strcpy(info->service, options.code.c_str());
            strcpy(info->service_instance, instance.c_str());
            std::thread h(heartbeat, options, instance);
            h.detach();
        }
        status = rc.ok();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

[[noreturn]] void Manager::heartbeat(const ManagerOptions &options, const std::string &serviceInstance) {
    std::shared_ptr<grpc::Channel> channel(grpc::CreateChannel(options.grpc, getCredentials(options)));
    std::unique_ptr<ManagementService::Stub> stub(ManagementService::NewStub(channel));

    while (true) {
        grpc::ClientContext context;
        InstancePingPkg ping;
        Commands commands;
        if (!options.authentication.empty()) {
            context.AddMetadata("authentication", options.authentication);
        }
        ping.set_service(options.code);
        ping.set_serviceinstance(serviceInstance);

        stub->keepAlive(&context, ping, &commands);

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

[[noreturn]] void Manager::consumer(const ManagerOptions &options) {
    while (true) {
        std::shared_ptr<grpc::Channel> channel(grpc::CreateChannel(options.grpc, getCredentials(options)));
        std::unique_ptr<TraceSegmentReportService::Stub> stub(TraceSegmentReportService::NewStub(channel));
        grpc::ClientContext context;
        Commands commands;

        if (!options.authentication.empty()) {
            context.AddMetadata("authentication", options.authentication);
        }
        auto writer = stub->collect(&context, &commands);

        try {
            boost::interprocess::message_queue mq(boost::interprocess::open_only, s_info->mq_name);

            while (true) {
                std::string data;
                data.resize(SKYWALKING_G(mq_max_message_length));
                size_t msg_size;
                unsigned msg_priority;
                mq.receive(&data[0], data.size(), msg_size, msg_priority);
                data.resize(msg_size);

                std::string json_str;
                SegmentObject msg;
                msg.ParseFromString(data);
                google::protobuf::util::JsonPrintOptions opt;
                opt.always_print_primitive_fields = true;
                opt.preserve_proto_field_names = true;
                google::protobuf::util::MessageToJsonString(msg, &json_str, opt);
                bool status = writer->Write(msg);
                if (status) {
                    sky_log("write success " + json_str);
                } else {
                    sky_log("write fail " + json_str);
                    break;
                }
            }
        } catch (boost::interprocess::interprocess_exception &ex) {
            sky_log(ex.what());
            php_error(E_WARNING, "%s %s", "[skywalking] open queue fail ", ex.what());
        }
    }
}

std::vector<std::string> Manager::getIps() {

    std::vector<std::string> ips;

    struct ifaddrs *interfaces = nullptr;
    struct ifaddrs *tempAddress = nullptr;
    int success = getifaddrs(&interfaces);

    if (success == 0) {
        tempAddress = interfaces;
        while (tempAddress != nullptr) {
            if (tempAddress->ifa_addr->sa_family == AF_INET) {
                std::string ip = inet_ntoa(((struct sockaddr_in *) tempAddress->ifa_addr)->sin_addr);
                if (ip.find("127") != 0) {
                    ips.push_back(ip);
                }
            }
            tempAddress = tempAddress->ifa_next;
        }
    }

    freeifaddrs(interfaces);

    return ips;
}

std::shared_ptr<grpc::ChannelCredentials> Manager::getCredentials(const ManagerOptions &options) {
    std::shared_ptr<grpc::ChannelCredentials> creds;
    if (options.grpc_tls == true) {
        if (options.cert_chain.empty() && options.private_key.empty()) {
            creds = grpc::SslCredentials(grpc::SslCredentialsOptions());
        } else {
            grpc::SslCredentialsOptions opts;
            opts.pem_cert_chain = options.cert_chain;
            opts.pem_root_certs = options.root_certs;
            opts.pem_private_key = options.private_key;
            creds = grpc::SslCredentials(opts);
        }
    } else {
        creds = grpc::InsecureChannelCredentials();
    }
    return creds;
}

std::string Manager::generateUUID() {

    static std::random_device dev;
    static std::mt19937 rng(dev());
    std::uniform_int_distribution<int> dist(0, 15);

    const char *v = "0123456789abcdef";
    const bool dash[] = {0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0};

    std::string res;
    for (bool i : dash) {
        if (i) res += "-";
        res += v[dist(rng)];
        res += v[dist(rng)];
    }

    return res;
}
