/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>

#include "grpc/ApplicationRegisterService.grpc.pb.h"
#include "grpc/DiscoveryService.grpc.pb.h"
#include "grpc/TraceSegmentService.grpc.pb.h"
#include "grpc/register/Register.grpc.pb.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;


extern "C" int applicationCodeRegister(char *grpc_server, char *code);

extern "C" int
registerInstance(char *grpc_server, int appId, char *uuid, long registertime, char *osname, char *hostname,
                 int processno, char *ipv4s);
extern "C" char* uuid();

class GreeterClient {
public:
    GreeterClient(std::shared_ptr <Channel> channel) {
        channel_ = channel;
    }

    int applicationCodeRegister(const std::string &code) {

        std::unique_ptr <ApplicationRegisterService::Stub> stub_;
        stub_ = ApplicationRegisterService::NewStub(channel_);

        Application request;

        request.set_applicationcode(code);

        ApplicationMapping reply;

        ClientContext context;

        Status status = stub_->applicationCodeRegister(&context, request, &reply);

        if (status.ok()) {
            if (reply.has_application()) {
                return reply.application().value();
            }
            return -100000;
        }

        return -100000;
    }

    int registerInstance(int applicationid, char *uuid, long registertime, char *osname, char *hostname, int processno,
                         char *ipv4s) {

        std::unique_ptr <InstanceDiscoveryService::Stub> stub_;
        stub_ = InstanceDiscoveryService::NewStub(channel_);

        ApplicationInstance request;


        request.set_agentuuid(uuid);
        request.set_applicationid(applicationid);
        request.set_registertime(registertime);

        OSInfo *osInfo = new OSInfo;

        request.set_allocated_osinfo(osInfo);
        osInfo->set_osname(osname);
        osInfo->set_hostname(hostname);
        osInfo->set_processno(processno);
        osInfo->add_ipv4s(ipv4s);


        ApplicationInstanceMapping reply;

        ClientContext context;

        Status status = stub_->registerInstance(&context, request, &reply);

        if (status.ok()) {
            if (reply.applicationinstanceid() != 0) {
                return reply.applicationinstanceid();
            }
            return -100000;
        }

        return -100000;

    }

private:
    std::shared_ptr <Channel> channel_;
};


int applicationCodeRegister(char *grpc_server, char *code) {
    GreeterClient greeter(grpc::CreateChannel(grpc_server, grpc::InsecureChannelCredentials()));

    std::string c(code);
//
    return greeter.applicationCodeRegister(c);
}

int
registerInstance(char *grpc_server, int appId, char *uuid, long registertime, char *osname, char *hostname,
                 int processno, char *ipv4s) {
    GreeterClient greeter(grpc::CreateChannel(grpc_server, grpc::InsecureChannelCredentials()));
    return greeter.registerInstance(appId, uuid, registertime, osname, hostname, processno, ipv4s);
}

char *uuid() {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    std::string str = boost::uuids::to_string(uuid);
    char *cstr = new char[str.length() + 1];
    strcpy(cstr, str.c_str());
    return cstr;
}
