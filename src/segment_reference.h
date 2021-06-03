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



#ifndef SKYWALKING_SEGMENT_REFERENCE_H
#define SKYWALKING_SEGMENT_REFERENCE_H


#include <string>

class SkySegmentReference {
public:

    int getRefType() const;

    const std::string &getTraceId() const;

    const std::string &getParentTraceSegmentId() const;

    int getParentSpanId() const;

    const std::string &getParentService() const;

    const std::string &getParentServiceInstance() const;

    const std::string &getParentEndpoint() const;

    const std::string &getNetworkAddressUsedAtPeer() const;

    void setRefType(int refType);

    void setTraceId(const std::string &traceId);

    void setParentTraceSegmentId(const std::string &parentTraceSegmentId);

    void setParentSpanId(int parentSpanId);

    void setParentService(const std::string &parentService);

    void setParentServiceInstance(const std::string &parentServiceInstance);

    void setParentEndpoint(const std::string &parentEndpoint);

    void setNetworkAddressUsedAtPeer(const std::string &networkAddressUsedAtPeer);

private:
    int refType;
    std::string traceId;
    std::string parentTraceSegmentId;
    int parentSpanId;
    std::string parentService;
    std::string parentServiceInstance;
    std::string parentEndpoint;
    std::string networkAddressUsedAtPeer;
};


#endif //SKYWALKING_SEGMENT_REFERENCE_H
