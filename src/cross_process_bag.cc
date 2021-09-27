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



#include "cross_process_bag.h"
#include "base64.h"
#include <regex>
#include <iostream>
#include <utility>

CrossProcessBag::CrossProcessBag(std::string serviceId,
                                 std::string serviceInstanceId,
                                 std::string segmentId,
                                 int version,
                                 const std::string &header) : version(version),
                                 currentTraceId(std::move(segmentId)),
                                 currentServiceId(std::move(serviceId)),
                                 currentServiceInstance(std::move(serviceInstanceId))
{
    sample = 0;
    parentSpanId = 0;
    decode(header);
}

void CrossProcessBag::decode(const std::string &h) {

    std::regex ws_re;
    switch (version) {
        case VERSION_8:
            ws_re = "-";
            break;
    }


    std::vector<std::string> header_bag(std::sregex_token_iterator(h.begin(), h.end(), ws_re, -1),
                                        std::sregex_token_iterator());

    if (header_bag.size() >= 8) {
        sample = std::stoi(header_bag[0]);
        traceId = Base64::decode(header_bag[1]);
        parentSegmentId = Base64::decode(header_bag[2]);
        parentSpanId = std::stoi(header_bag[3]);
        parentService = Base64::decode(header_bag[4]);
        parentServiceInstance = Base64::decode(header_bag[5]);
        parentEndpoint = Base64::decode(header_bag[6]);
        networkAddressUsedAtPeer = Base64::decode(header_bag[7]);
    } else {
        traceId = currentTraceId;
    }
}

std::string CrossProcessBag::encode(int spanId, const std::string &peer) {

    std::vector<std::string> tmp;
    tmp.emplace_back("1");
    tmp.emplace_back(Base64::encode(traceId));
    tmp.emplace_back(Base64::encode(currentTraceId));
    tmp.emplace_back(std::to_string(spanId));
    tmp.emplace_back(Base64::encode(currentServiceId));
    tmp.emplace_back(Base64::encode(currentServiceInstance));
    tmp.emplace_back(Base64::encode(currentOperationName));
    tmp.emplace_back(Base64::encode(peer));

    std::string header;

    for (const auto &val:tmp) {
        header.append(val);
        header.append("-");
    }

    header.erase(header.end() - 1);

    return header;
}

void CrossProcessBag::setOperationName(const std::string &name) {
    currentOperationName = name;
}

const std::string &CrossProcessBag::getTraceId() {
    return traceId;
}

const std::string &CrossProcessBag::getParentTraceSegmentId() {
    return parentSegmentId;
}

int CrossProcessBag::getParentSpanId() const {
    return parentSpanId;
}

const std::string &CrossProcessBag::getParentService() {
    return parentService;
}

const std::string &CrossProcessBag::getParentServiceInstance() {
    return parentServiceInstance;
}

const std::string &CrossProcessBag::getParentEndpoint() {
    return parentEndpoint;
}

const std::string &CrossProcessBag::getNetworkAddressUsedAtPeer() {
    return networkAddressUsedAtPeer;
}