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


extern "C" int applicationCodeRegister(char *grpc_server, char *service_name);

extern "C" int
registerInstance(char *grpc_server, int appId, long registertime, char *osname, char *hostname,
                 int processno, char *ipv4s);

static boost::uuids::uuid uuid = boost::uuids::random_generator()();

class GreeterClient {
public:
    GreeterClient(std::shared_ptr<Channel> channel)
        : stub_(Register::NewStub(channel)) {}

    int applicationCodeRegister(const std::string &service_name) {
        Services request;

        Service *s = request.add_services();
        s->set_servicename(service_name);

        ServiceRegisterMapping reply;

        ClientContext context;

        Status status = stub_->doServiceRegister(&context, request, &reply);

        if (status.ok()) {
            for (int i = 0; i < reply.services_size(); i++) {
                const KeyIntValuePair &kv = reply.services(i);
                std::cout << "Register Service:" << std::endl;
                std::cout << kv.key() << ": " << kv.value() << std::endl;
                if (kv.key() == service_name) {
                    return kv.value();
                }
            }
        }

        return -100000;
    }

    int registerInstance(int applicationid, long registertime, char *osname, char *hostname, int processno,
                         char *ipv4s) {
        ServiceInstances request;
        ServiceInstance *s = request.add_instances();


        s->set_serviceid(applicationid);
        s->set_instanceuuid(boost::uuids::to_string(uuid));
        s->set_time(registertime);

        KeyStringValuePair *os = s->add_properties();
        KeyStringValuePair *host = s->add_properties();
        KeyStringValuePair *process = s->add_properties();
        KeyStringValuePair *ipv4 = s->add_properties();
        KeyStringValuePair *language = s->add_properties();

        os->set_key("os_name");
        os->set_value(osname);
        host->set_key("host_name");
        host->set_value(hostname);
        process->set_key("process_no");
        process->set_value(std::to_string(processno));
        ipv4->set_key("ipv4");
        ipv4->set_value(ipv4s);
        language->set_key("language");
        language->set_value("php");

        ServiceInstanceRegisterMapping reply;

        ClientContext context;

        Status status = stub_->doServiceInstanceRegister(&context, request, &reply);

        std::cout << "Register Instance: " << status.ok() << " applicationid: " << applicationid << " service size: " << reply.serviceinstances_size() << std::endl;

        if (status.ok()) {
            for (int i = 0; i < reply.serviceinstances_size(); i++) {
                const KeyIntValuePair &kv = reply.serviceinstances(i);
                std::cout << "Register Instance:"<< std::endl;
                std::cout << kv.key() << ": " << kv.value() << std::endl;

                if (kv.key() == boost::uuids::to_string(uuid)) {
                    std::cout << "uuid" << ": " << boost::uuids::to_string(uuid) << std::endl;
                    return kv.value();
                }
            }
        }

        return -100000;

    }

private:
    std::unique_ptr<Register::Stub> stub_;
};


int applicationCodeRegister(char *grpc_server, char *service_name) {
    GreeterClient greeter(grpc::CreateChannel(grpc_server, grpc::InsecureChannelCredentials()));

    std::string c(service_name);

    return greeter.applicationCodeRegister(c);
}

int
registerInstance(char *grpc_server, int appId, long registertime, char *osname, char *hostname,
                 int processno, char *ipv4s) {
    GreeterClient greeter(grpc::CreateChannel(grpc_server, grpc::InsecureChannelCredentials()));
    return greeter.registerInstance(appId, registertime, osname, hostname, processno, ipv4s);
}
