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
#include <netdb.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <sstream>
#include <random>
#include <fstream>
#include <queue>
#include "manager_wrapper.h"
#include "management/Management.grpc.pb.h"
#include "language-agent/Tracing.grpc.pb.h"
#include "grpc/grpc.h"
#include "grpc++/grpc++.h"
#include "segment.h"
#include <google/protobuf/util/json_util.h>


std::queue<std::string> messageQueue;
static pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t cond_mx = PTHREAD_MUTEX_INITIALIZER;

Manager::Manager(int version, const std::string &code, const std::string &grpc, struct service_info *info, int *fd) {
    std::thread th(login, version, code, grpc, info, fd);
    th.detach();

    std::thread c(consumer, fd);
    c.detach();

    std::thread s(sender, grpc);
    s.detach();

    std::cout << "readddd" << std::endl;
}

void Manager::login(int version, const std::string &code, const std::string &grpc, struct service_info *info, int *fd) {

    std::ofstream fout("/tmp/log.txt");
    std::shared_ptr<grpc::Channel> channel(grpc::CreateChannel(grpc, grpc::InsecureChannelCredentials()));
    std::unique_ptr<ManagementService::Stub> stub(ManagementService::NewStub(channel));

    bool status = false;

    while (!status) {
        grpc::ClientContext context;
        InstanceProperties properties;
        Commands commands;

        auto ips = getIps();

        std::string instance;
        if (!ips.empty()) {
            // todo port
            instance = generateUUID() + "@" + ips[0];
        }

        properties.set_service(code);
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
        if (rc.ok()) {
            strcpy(info->service, code.c_str());
            strcpy(info->service_instance, instance.c_str());
            std::thread h(heartbeat, grpc, code, instance);
            h.detach();
        }
        status = rc.ok();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

[[noreturn]] void Manager::heartbeat(const std::string &grpc, const std::string &service, const std::string &serviceInstance) {
    std::shared_ptr<grpc::Channel> channel(grpc::CreateChannel(grpc, grpc::InsecureChannelCredentials()));
    std::unique_ptr<ManagementService::Stub> stub(ManagementService::NewStub(channel));

    while (true) {
        grpc::ClientContext context;
        InstancePingPkg ping;
        Commands commands;

        ping.set_service(service);
        ping.set_serviceinstance(serviceInstance);

        stub->keepAlive(&context, ping, &commands);

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

[[noreturn]] void Manager::consumer(int *fd) {
    while (true) {
        char buffer[2048];
        int len = read(fd[0], buffer, 2048);
        if (len > 0) {
            pthread_mutex_lock(&mx);
            std::string msg(buffer, len);
            messageQueue.push(msg);
            pthread_mutex_unlock(&mx);

            pthread_mutex_lock(&cond_mx);
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&cond_mx);
        }
    }
}

[[noreturn]] void Manager::sender(const std::string &grpc) {
    std::ofstream fout("/tmp/sender.txt");

    std::shared_ptr<grpc::Channel> channel(grpc::CreateChannel(grpc, grpc::InsecureChannelCredentials()));
    std::unique_ptr<TraceSegmentReportService::Stub> stub(TraceSegmentReportService::NewStub(channel));
    grpc::ClientContext context;
    Commands commands;
    auto writer = stub->collect(&context, &commands);

    while (true) {
        pthread_mutex_lock(&cond_mx);
        pthread_cond_wait(&cond, &cond_mx);
        pthread_mutex_unlock(&cond_mx);

        while (!messageQueue.empty()) {
            pthread_mutex_lock(&mx);
            std::string data = messageQueue.front();
            messageQueue.pop();
            pthread_mutex_unlock(&mx);

            // todo sender
            std::string json_str;
            fout << "sender" << std::endl;
            SegmentObject msg;
            msg.ParseFromString(data);
            google::protobuf::util::JsonPrintOptions opt;
            opt.always_print_primitive_fields = true;
            opt.preserve_proto_field_names = true;
            google::protobuf::util::MessageToJsonString(msg, &json_str, opt);
            writer->Write(msg);
            fout << json_str << std::endl;
            fout << "sender111" << std::endl;
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