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



#include "sky_core_segment_reference.h"

// get
SkyCoreRefType SkyCoreSegmentReference::getRefType() const {
    return refType;
}

const std::string &SkyCoreSegmentReference::getTraceId() const {
    return traceId;
}

const std::string &SkyCoreSegmentReference::getParentTraceSegmentId() const {
    return parentTraceSegmentId;
}

int SkyCoreSegmentReference::getParentSpanId() const {
    return parentSpanId;
}

const std::string &SkyCoreSegmentReference::getParentService() const {
    return parentService;
}

const std::string &SkyCoreSegmentReference::getParentServiceInstance() const {
    return parentServiceInstance;
}

const std::string &SkyCoreSegmentReference::getParentEndpoint() const {
    return parentEndpoint;
}

const std::string &SkyCoreSegmentReference::getNetworkAddressUsedAtPeer() const {
    return networkAddressUsedAtPeer;
}

// set
void SkyCoreSegmentReference::setRefType(SkyCoreRefType refType) {
    SkyCoreSegmentReference::refType = refType;
}

void SkyCoreSegmentReference::setTraceId(const std::string &traceId) {
    SkyCoreSegmentReference::traceId = traceId;
}

void SkyCoreSegmentReference::setParentTraceSegmentId(const std::string &parentTraceSegmentId) {
    SkyCoreSegmentReference::parentTraceSegmentId = parentTraceSegmentId;
}

void SkyCoreSegmentReference::setParentSpanId(int parentSpanId) {
    SkyCoreSegmentReference::parentSpanId = parentSpanId;
}

void SkyCoreSegmentReference::setParentService(const std::string &parentService) {
    SkyCoreSegmentReference::parentService = parentService;
}

void SkyCoreSegmentReference::setParentServiceInstance(const std::string &parentServiceInstance) {
    SkyCoreSegmentReference::parentServiceInstance = parentServiceInstance;
}

void SkyCoreSegmentReference::setParentEndpoint(const std::string &parentEndpoint) {
    SkyCoreSegmentReference::parentEndpoint = parentEndpoint;
}

void SkyCoreSegmentReference::setNetworkAddressUsedAtPeer(const std::string &networkAddressUsedAtPeer) {
    SkyCoreSegmentReference::networkAddressUsedAtPeer = networkAddressUsedAtPeer;
}
