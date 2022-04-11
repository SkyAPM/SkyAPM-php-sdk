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
#include <regex>
#include <utility>

#include "base64.h"
#include "sky_go_wrapper.h"

SkyCoreCrossProcess::SkyCoreCrossProcess(const std::string &header, const std::string &segmentId) {
    std::regex re("-");
    std::vector<std::string> header_bag(std::sregex_token_iterator(header.begin(), header.end(), re, -1),
                                        std::sregex_token_iterator());

    if (header_bag.size() >= 8) {
        sample = std::stoi(header_bag[0]);
        traceId = Base64::decode(header_bag[1]);
        parentSegmentId = Base64::decode(header_bag[2]);
        parentSpanId = std::stoi(header_bag[3]);
        parentService = Base64::decode(header_bag[4]);
        parentServiceInstance = Base64::decode(header_bag[5]);
        parentEndpoint = Base64::decode(header_bag[6]);
        networkAddress = Base64::decode(header_bag[7]);
    } else {
        traceId = segmentId;
    }
}

std::string SkyCoreCrossProcess::encode(int spanId, const std::string &peer) {

    std::vector<std::string> tmp;
    tmp.emplace_back("1");
    tmp.emplace_back(Base64::encode(traceId));
    tmp.emplace_back(Base64::encode(_segmentId));
    tmp.emplace_back(std::to_string(spanId));
    tmp.emplace_back(Base64::encode(_service));
    tmp.emplace_back(Base64::encode(_serviceInstance));
    tmp.emplace_back(Base64::encode(_endpoint));
    tmp.emplace_back(Base64::encode(peer));

    std::string header;

    for (const auto &val: tmp) {
        header.append(val);
        header.append("-");
    }

    header.erase(header.end() - 1);

    return header;
}

void SkyCoreCrossProcess::setService(const std::string &service) {
    _service = service;
}

void SkyCoreCrossProcess::setServiceInstance(const std::string &serviceInstance) {
    _serviceInstance = serviceInstance;
}

void SkyCoreCrossProcess::setSegmentId(const std::string &segmentId) {
    _segmentId = segmentId;
}

void SkyCoreCrossProcess::setEndpoint(const std::string &endpoint) {
    _endpoint = endpoint;
}

const std::string &SkyCoreCrossProcess::getTraceId() {
    return traceId;
}

const std::string &SkyCoreCrossProcess::getParentTraceSegmentId() {
    return parentSegmentId;
}

int SkyCoreCrossProcess::getParentSpanId() const {
    return parentSpanId;
}

const std::string &SkyCoreCrossProcess::getParentService() {
    return parentService;
}

const std::string &SkyCoreCrossProcess::getParentServiceInstance() {
    return parentServiceInstance;
}

const std::string &SkyCoreCrossProcess::getParentEndpoint() {
    return parentEndpoint;
}

const std::string &SkyCoreCrossProcess::getNetworkAddress() {
    return networkAddress;
}