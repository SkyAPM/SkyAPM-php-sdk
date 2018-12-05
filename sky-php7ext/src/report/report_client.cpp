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
#include <fstream>
#include <memory>
#include <string>
#include <sys/types.h>
#include <dirent.h>
#include <regex>
#include <chrono>
#include <cstdio>
#include <thread>
#include "json.hpp"

#include <grpcpp/grpcpp.h>
#include <google/protobuf/util/json_util.h>


#include "../grpc/ApplicationRegisterService.grpc.pb.h"
#include "../grpc/DiscoveryService.grpc.pb.h"
#include "../grpc/TraceSegmentService.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientWriter;
using json = nlohmann::json;

class GreeterClient {
public:
    GreeterClient(std::shared_ptr<Channel> channel)
            : stub_(TraceSegmentService::NewStub(channel)) {}

    int collect(UpstreamSegment request) {


        Downstream reply;

        ClientContext context;
        std::unique_ptr<ClientWriter<UpstreamSegment>> writer(stub_->collect(&context, &reply));
        if (!writer->Write(request)) {
        }
        writer->WritesDone();
        Status status = writer->Finish();


        if (status.ok()) {
            std::cout << "send ok!" << std::endl;
        } else {
            std::cout << "send error!" << status.error_message() << std::endl;
        }

        return 1;
    }

private:
    std::unique_ptr<TraceSegmentService::Stub> stub_;
};

int main(int argc, char **argv) {


    for (int i = 0; i < argc; ++i) {
        if (std::strncmp("-h", argv[i], sizeof(argv[i]) - 1) == 0 ||
            std::strncmp("--help", argv[i], sizeof(argv[i]) - 1) == 0) {
            std::cout << "report_client grpc log_path" << std::endl;
            std::cout << "e.g. report_client 120.0.0.1:11800 /tmp" << std::endl;
            return 0;
        }
    }

    if (argc == 0) {
        std::cout << "report_client grpc log_path" << std::endl;
        std::cout << "e.g. report_client 120.0.0.1:11800 /tmp" << std::endl;
        return 0;
    }


    GreeterClient greeter(grpc::CreateChannel(argv[1], grpc::InsecureChannelCredentials()));

    while (1) {

        struct dirent *dir;
        DIR *dp;
        if ((dp = opendir(argv[2])) == NULL) {
            std::cerr << "open directory error";
            return 0;
        }

        while ((dir = readdir(dp)) != NULL) {
            if (strcmp(".", dir->d_name) == 0 || strcmp("..", dir->d_name) == 0) {
                continue;
            }

            std::string fileName = std::string(argv[2]) + "/" + dir->d_name;

            const std::regex pattern(std::string(argv[2]) + "/skywalking\\.(\\d+)\\.log");
            if (std::regex_match(fileName, pattern)) {
                std::match_results<std::string::const_iterator> result;

                bool valid = std::regex_match(fileName, result, pattern);

                if (valid) {

                    time_t t = time(NULL);
                    t = t - 65;
                    char ch[64] = {0};
                    strftime(ch, sizeof(ch) - 1, "%Y%m%d%H%M", localtime(&t));
                    unsigned long fileTime = std::stoul(result[1]);
                    unsigned long localTime = std::stoul(ch);

                    if (fileTime < localTime) {
                        std::ifstream file;
                        file.open(fileName, std::ios::in);
                        if (file.is_open()) {
                            std::cout << "send `" << fileName << "` to skywalking service" << std::endl;

                            std::string strLine;
                            while (std::getline(file, strLine))
                            {

                                if (strLine.empty()) {
                                    continue;
                                }
                                json j = json::parse(strLine);

                                UpstreamSegment request;

                                std::smatch traceResult;
                                std::string tmp(j["segment"]["traceSegmentId"].get<std::string>());
                                bool valid = std::regex_match(tmp,
                                                              traceResult, std::regex("([\\-0-9]+)\\.(\\d+)\\.(\\d+)"));

                                if (valid) {

                                    for (int i = 0; i < j["globalTraceIds"].size(); i++) {

                                        std::cout << "send " << j["globalTraceIds"][i].get<std::string>() << " to skywalking service"
                                                  << std::endl;
                                        std::smatch globalTraceResult;
                                        std::string tmp(j["globalTraceIds"][i].get<std::string>());

                                        bool valid = std::regex_match(tmp, globalTraceResult, std::regex("(\\-?\\d+)\\.(\\d+)\\.(\\d+)"));
                                        UniqueId *globalTrace = request.add_globaltraceids();

                                        long long idp1 = std::stoll(globalTraceResult[1]);
                                        long long idp2 = std::stoll(globalTraceResult[2]);
                                        long long idp3 = std::stoll(globalTraceResult[3]);
                                        globalTrace->add_idparts(idp1);
                                        globalTrace->add_idparts(idp2);
                                        globalTrace->add_idparts(idp3);
                                    }

                                    UniqueId *uniqueId = new UniqueId;
                                    long long idp1 = std::stoll(traceResult[1]);
                                    long long idp2 = std::stoll(traceResult[2]);
                                    long long idp3 = std::stoll(traceResult[3]);
                                    uniqueId->add_idparts(idp1);
                                    uniqueId->add_idparts(idp2);
                                    uniqueId->add_idparts(idp3);

                                    TraceSegmentObject traceSegmentObject;
                                    traceSegmentObject.set_allocated_tracesegmentid(uniqueId);
                                    traceSegmentObject.set_applicationid(j["application_id"].get<int>());
                                    traceSegmentObject.set_applicationinstanceid(j["application_instance"].get<int>());
                                    traceSegmentObject.set_issizelimited(j["segment"]["isSizeLimited"].get<int>());

                                    auto spans = j["segment"]["spans"];
                                    for (int i = 0; i < spans.size(); i++) {

                                        SpanObject *spanObject = traceSegmentObject.add_spans();
                                        spanObject->set_spanid(spans[i]["spanId"]);
                                        spanObject->set_parentspanid(spans[i]["parentSpanId"]);
                                        spanObject->set_starttime(spans[i]["startTime"]);
                                        spanObject->set_endtime(spans[i]["endTime"]);
                                        spanObject->set_operationname(spans[i]["operationName"]);

                                        int spanType = spans[i]["spanType"].get<int>();
                                        if (spanType == 0) {
                                            spanObject->set_spantype(SpanType::Entry);
                                        } else if (spanType == 2) {
                                            spanObject->set_spantype(SpanType::Local);
                                        } else if (spanType == 1) {
                                            spanObject->set_spantype(SpanType::Exit);
                                        }

                                        int spanLayer = spans[i]["spanLayer"].get<int>();
                                        if (spanLayer == 3) {
                                            spanObject->set_spanlayer(SpanLayer::Http);
                                        }

                                        spanObject->set_componentid(spans[i]["componentId"]);
                                        spanObject->set_iserror(spans[i]["isError"].get<int>());
                                    }

                                    std::string test;

                                    traceSegmentObject.SerializeToString(&test);
                                    request.set_segment(test);
                                    greeter.collect(request);
                                }
                            }
                            remove(fileName.c_str());
                        }
                    }
                }

            }
        }
        closedir(dp);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }


//    std::string user("world");
//    std::string reply = greeter.SayHello(user);
//    std::cout << "Greeter received: " << reply << std::endl;

    return 0;
}