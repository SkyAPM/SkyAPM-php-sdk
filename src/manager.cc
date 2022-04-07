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
#include "segment.h"
#include "common.h"
#include "sky_shm.h"
#include <boost/interprocess/ipc/message_queue.hpp>

#include "php_skywalking.h"
#include "sky_log.h"
#include "protocol.h"

std::queue<std::string> messageQueue;
static pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t cond_mx = PTHREAD_MUTEX_INITIALIZER;

extern struct service_info *s_info;

void Manager::init(const ManagerOptions &options, struct service_info *info) {
    std::thread th(reportInstance, options, info);
    th.detach();

    std::thread c(consumer, options);
    c.detach();

    sky_log("the apache skywalking php plugin mounted");
}

void Manager::reportInstance(const ManagerOptions &options, struct service_info *info) {

    bool done = false;
    while (!done) {
        GoString address = {options.grpc.c_str(), static_cast<ptrdiff_t>(options.grpc.size())};
        GoString server = {options.code.c_str(), static_cast<ptrdiff_t>(options.code.size())};
        GoString instance = {options.instance_name.c_str(), static_cast<ptrdiff_t>(options.instance_name.size())};
        ReportInstanceProperties_return res = ReportInstanceProperties(address, server, instance);

        if (res.r1.n == 0 && info != nullptr) {
            strcpy(info->service, options.code.c_str());
            strcpy(info->service_instance, res.r0.p);
            done = true;
        }
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
