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
#include <sys/time.h>
#if !_MSC_VER && !__clang__ && (__GNUC__ < 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ <= 8)))
#include <boost/regex.hpp>
using boost::regex;
using boost::regex_match;
using boost::match_results;
using boost::smatch;
#else
#include <regex>
using std::regex;
using std::regex_match;
using std::match_results;
using std::smatch;
#endif
#include <chrono>
#include <cstdio>
#include <thread>
#include "json.hpp"
#include <signal.h>

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
            : stub_(TraceSegmentService::NewStub(channel)), discoveryStub_(InstanceDiscoveryService::NewStub(channel)) {}

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

    int heartbeat(ApplicationInstanceHeartbeat request) {
        Downstream reply;

        ClientContext context;

        Status status = discoveryStub_->heartbeat(&context, request, &reply);
        if (status.ok()) {
            std::cout << "send heartbeat ok!" << std::endl;
        } else {
            std::cout << "send heartbeat error!" << status.error_message() << std::endl;
        }

        return 1;
    }

private:
    std::unique_ptr<TraceSegmentService::Stub> stub_;
    std::unique_ptr<InstanceDiscoveryService::Stub> discoveryStub_;
};

int main(int argc, char **argv) {


    for (int i = 0; i < argc; ++i) {
        if (std::strncmp("-h", argv[i], sizeof(argv[i]) - 1) == 0 ||
            std::strncmp("--help", argv[i], sizeof(argv[i]) - 1) == 0) {
            std::cout << "report_client grpc log_path" << std::endl;
            std::cout << "e.g. report_client 127.0.0.1:11800 /tmp" << std::endl;
            return 0;
        }
    }

    if (argc == 1) {
        std::cout << "report_client grpc log_path" << std::endl;
        std::cout << "e.g. report_client 127.0.0.1:11800 /tmp" << std::endl;
        return 0;
    }


    GreeterClient greeter(grpc::CreateChannel(argv[1], grpc::InsecureChannelCredentials()));
    std::map<int, int> appInstances;
    std::map<int, long> sendTime;

    while (1) {

        struct dirent *dir;
        DIR *dp;
        if ((dp = opendir(argv[2])) == NULL) {
            std::cerr << "open directory error";
            return 0;
        }

        // heartbeat
        for (auto &i: appInstances) {

            struct timeval tv;
            gettimeofday(&tv, NULL);

            if(tv.tv_sec - sendTime[i.first] > 40) {
                if (0 == kill(appInstances[i.first], 0)) {
                    sendTime[i.first] = tv.tv_sec;
                    std::cout << "send heartbeat ..." << std::endl;
                    ApplicationInstanceHeartbeat request;
                    request.set_applicationinstanceid(i.first);
                    request.set_heartbeattime(tv.tv_sec*1000 + tv.tv_usec/1000);
                    greeter.heartbeat(request);
                }
            }
        }

        while ((dir = readdir(dp)) != NULL) {
            if (strcmp(".", dir->d_name) == 0 || strcmp("..", dir->d_name) == 0) {
                continue;
            }

            std::string fileName = std::string(argv[2]) + "/" + dir->d_name;

            const regex pattern(std::string(argv[2]) + "/skywalking\\.(\\d+)-\\d+\\.log");
            if (regex_match(fileName, pattern)) {
                match_results<std::string::const_iterator> result;

                bool valid = regex_match(fileName, result, pattern);

                if (valid) {

                    struct timeval tv;
                    gettimeofday(&tv,NULL);

                    long fileTime = std::stol(result[1]);
                    long localTime = tv.tv_sec - 3;

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

                                smatch traceResult;
                                std::string tmp(j["segment"]["traceSegmentId"].get<std::string>());
                                bool valid = regex_match(tmp,
                                                              traceResult, regex("([\\-0-9]+)\\.(\\d+)\\.(\\d+)"));

                                if (valid) {
                                    // add to map
                                    if(!appInstances[j["application_instance"]]) {
                                        appInstances[j["application_instance"]] = j["pid"];
                                        sendTime[j["application_instance"]] = 0;
                                    }


                                    for (int i = 0; i < j["globalTraceIds"].size(); i++) {

                                        std::cout << "send " << j["globalTraceIds"][i].get<std::string>() << " to skywalking service"
                                                  << std::endl;
                                        smatch globalTraceResult;
                                        std::string tmp(j["globalTraceIds"][i].get<std::string>());

                                        bool valid = regex_match(tmp, globalTraceResult, regex("(\\-?\\d+)\\.(\\d+)\\.(\\d+)"));
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
                                        spanObject->set_spanid(spans[i]["spanId"].get<int>());
                                        spanObject->set_parentspanid(spans[i]["parentSpanId"].get<int>());
                                        spanObject->set_starttime(spans[i]["startTime"]);
                                        spanObject->set_endtime(spans[i]["endTime"]);
                                        spanObject->set_operationname(spans[i]["operationName"]);
                                        std::string peer(spans[i]["peer"].get<std::string>());
                                        if(!peer.empty()) {
                                            spanObject->set_peer(peer);
                                        }
                                        KeyWithStringValue *kv = spanObject->add_tags();
                                        kv->set_key("url");
                                        kv->set_value(spans[i]["tags"]["url"]);

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

                                        spanObject->set_componentid(spans[i]["componentId"].get<int>());
                                        spanObject->set_iserror(spans[i]["isError"].get<int>());

                                        // refs
                                        auto refs = spans[i]["refs"];
                                        for (int k = 0; k < refs.size(); k++) {

                                            smatch traceResult;
                                            std::string tmp(refs[k]["parentTraceSegmentId"].get<std::string>());
                                            bool valid = regex_match(tmp,
                                                                          traceResult, regex("([\\-0-9]+)\\.(\\d+)\\.(\\d+)"));

                                            UniqueId *uniqueIdTmp = new UniqueId;
                                            long long idp1 = std::stoll(traceResult[1]);
                                            long long idp2 = std::stoll(traceResult[2]);
                                            long long idp3 = std::stoll(traceResult[3]);
                                            uniqueIdTmp->add_idparts(idp1);
                                            uniqueIdTmp->add_idparts(idp2);
                                            uniqueIdTmp->add_idparts(idp3);

                                            TraceSegmentReference *r = spanObject->add_refs();
                                            r->set_allocated_parenttracesegmentid(uniqueIdTmp);
                                            r->set_parentspanid(refs[k]["parentSpanId"].get<int>());
                                            r->set_parentapplicationinstanceid(refs[k]["parentApplicationInstanceId"].get<int>());
                                            r->set_networkaddress(refs[k]["networkAddress"].get<std::string>());
                                            r->set_entryapplicationinstanceid(refs[k]["entryApplicationInstanceId"].get<int>());
                                            r->set_entryservicename(refs[k]["entryServiceName"].get<std::string>());
                                            r->set_parentservicename(refs[k]["parentServiceName"].get<std::string>());
                                        }
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