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


#include "sky_core_cross_process.h"
#include "php.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "sky_util_base64.h"

sky_core_cross_process_t *sky_core_cross_process_new(char *header) {
    sky_core_cross_process_t *cross_process = (sky_core_cross_process_t *) emalloc(sizeof(sky_core_cross_process_t));
    memset(cross_process, 0, sizeof(sky_core_cross_process_t));

    if (header != NULL) {
        char **bag = (char **) malloc(8);
        char *p = strtok(header, "-");
        int i = 0;
        while (p != NULL) {
            asprintf(&bag[i], "%s", p);
            i++;
            p = strtok(NULL, "-");
        }

        if (i >= 8) {
            cross_process->sample = (int) strtol(bag[0], NULL, 10);
            cross_process->traceId = sky_util_base64_decode(bag[1]);
            cross_process->parentSegmentId = sky_util_base64_decode(bag[2]);
            cross_process->parentSpanId = (int) strtol(bag[3], NULL, 10);
            cross_process->parentService = sky_util_base64_decode(bag[4]);
            cross_process->parentServiceInstance = sky_util_base64_decode(bag[5]);
            cross_process->parentEndpoint = sky_util_base64_decode(bag[6]);
            cross_process->networkAddress = sky_util_base64_decode(bag[7]);
        }
    }

    return cross_process;
}

void sky_core_cross_process_set_trace_id(sky_core_cross_process_t *cross_process, char *trace_id) {
    if (cross_process->traceId == NULL) {
        cross_process->traceId = (char *) emalloc(strlen(trace_id) + 1);
        bzero(cross_process->traceId, strlen(trace_id) + 1);
        memcpy(cross_process->traceId, trace_id, strlen(trace_id));
    }
}

//std::string SkyCoreCrossProcess::encode(int spanId, const std::string &peer) {
//
//    std::vector<std::string> tmp;
//    tmp.emplace_back("1");
//    tmp.emplace_back(Base64::encode(traceId));
//    tmp.emplace_back(Base64::encode(_segmentId));
//    tmp.emplace_back(std::to_string(spanId));
//    tmp.emplace_back(Base64::encode(_service));
//    tmp.emplace_back(Base64::encode(_serviceInstance));
//    tmp.emplace_back(Base64::encode(_endpoint));
//    tmp.emplace_back(Base64::encode(peer));
//
//    std::string header;
//
//    for (const auto &val: tmp) {
//        header.append(val);
//        header.append("-");
//    }
//
//    header.erase(header.end() - 1);
//
//    return header;
//}
//
//void SkyCoreCrossProcess::setService(const std::string &service) {
//    _service = service;
//}
//
//void SkyCoreCrossProcess::setServiceInstance(const std::string &serviceInstance) {
//    _serviceInstance = serviceInstance;
//}
//
//void SkyCoreCrossProcess::setSegmentId(const std::string &segmentId) {
//    _segmentId = segmentId;
//}
//
//void SkyCoreCrossProcess::setEndpoint(const std::string &endpoint) {
//    _endpoint = endpoint;
//}
//
//const std::string &SkyCoreCrossProcess::getTraceId() {
//    return traceId;
//}
//
//const std::string &SkyCoreCrossProcess::getParentTraceSegmentId() {
//    return parentSegmentId;
//}
//
//int SkyCoreCrossProcess::getParentSpanId() const {
//    return parentSpanId;
//}
//
//const std::string &SkyCoreCrossProcess::getParentService() {
//    return parentService;
//}
//
//const std::string &SkyCoreCrossProcess::getParentServiceInstance() {
//    return parentServiceInstance;
//}
//
//const std::string &SkyCoreCrossProcess::getParentEndpoint() {
//    return parentEndpoint;
//}
//
//const std::string &SkyCoreCrossProcess::getNetworkAddress() {
//    return networkAddress;
//}