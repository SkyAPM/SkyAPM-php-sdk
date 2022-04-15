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



#ifndef SKYWALKING_SKY_CORE_CROSS_PROCESS_H
#define SKYWALKING_SKY_CORE_CROSS_PROCESS_H

typedef struct sky_core_cross_process_t {

    // https://github.com/apache/skywalking/blob/master/docs/en/protocols/Skywalking-Cross-Process-Propagation-Headers-Protocol-v3.md
    int sample;
    char *traceId;
    char *parentSegmentId;
    int parentSpanId;
    char *parentService;
    char *parentServiceInstance;
    char *parentEndpoint;
    char *networkAddress;
} sky_core_cross_process_t;

sky_core_cross_process_t *sky_core_cross_process_new(char *header);

void sky_core_cross_process_set_trace_id(sky_core_cross_process_t *cross_process, char *trace_id);

//#include <string>
//
//class SkyCoreCrossProcess {
//public:
//    SkyCoreCrossProcess(const std::string &header, const std::string &segmentId);
//
//    std::string encode(int spanId, const std::string &peer);
//
//    void setService(const std::string &service);
//
//    void setServiceInstance(const std::string &serviceInstance);
//
//    void setSegmentId(const std::string &segmentId);
//
//    void setEndpoint(const std::string &endpoint);
//
//    const std::string &getTraceId();
//
//    const std::string &getParentTraceSegmentId();
//
//    int getParentSpanId() const;
//
//    const std::string &getParentService();
//
//    const std::string &getParentServiceInstance();
//
//    const std::string &getParentEndpoint();
//
//    const std::string &getNetworkAddress();
//
//private:
//    std::string _service;
//    std::string _serviceInstance;
//    std::string _segmentId;
//    std::string _endpoint;
//
//    // https://github.com/apache/skywalking/blob/master/docs/en/protocols/Skywalking-Cross-Process-Propagation-Headers-Protocol-v3.md
//    int sample;
//    std::string traceId;
//    std::string parentSegmentId;
//    int parentSpanId;
//    std::string parentService;
//    std::string parentServiceInstance;
//    std::string parentEndpoint;
//    std::string networkAddress;
//};


#endif //SKYWALKING_SKY_CORE_CROSS_PROCESS_H
