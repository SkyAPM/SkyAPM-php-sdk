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

#ifndef SKYWALKING_CROSS_PROCESS_BAG_H
#define SKYWALKING_CROSS_PROCESS_BAG_H

#define VERSION_8 8

#include <string>

class CrossProcessBag {
public:
    CrossProcessBag(std::string serviceId,
                    std::string serviceInstanceId, std::string segmentId,
                    int version,
                    const std::string &header);

    std::string encode(int spanId, const std::string &peer);

    void setOperationName(std::string name);

    std::string getTraceId();

    std::string getParentTraceSegmentId();

    int getParentSpanId() const;

    std::string getParentService();

    std::string getParentServiceInstance();

    std::string getParentEndpoint();

    std::string getNetworkAddressUsedAtPeer();

private:
    void decode(const std::string &h);

private:
    int version;
    std::string currentTraceId;
    std::string currentServiceId;
    std::string currentServiceInstance;
    std::string currentOperationName;

    int sample;
    std::string traceId;
    std::string parentSegmentId;
    int parentSpanId;
    std::string parentService;
    std::string parentServiceInstance;
    std::string parentEndpoint;
    std::string networkAddressUsedAtPeer;
};


#endif //SKYWALKING_CROSS_PROCESS_BAG_H
