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



#include "segment_reference.h"

// get
int SkySegmentReference::getRefType() const {
    return refType;
}

const std::string &SkySegmentReference::getTraceId() const {
    return traceId;
}

const std::string &SkySegmentReference::getParentTraceSegmentId() const {
    return parentTraceSegmentId;
}

int SkySegmentReference::getParentSpanId() const {
    return parentSpanId;
}

const std::string &SkySegmentReference::getParentService() const {
    return parentService;
}

const std::string &SkySegmentReference::getParentServiceInstance() const {
    return parentServiceInstance;
}

const std::string &SkySegmentReference::getParentEndpoint() const {
    return parentEndpoint;
}

const std::string &SkySegmentReference::getNetworkAddressUsedAtPeer() const {
    return networkAddressUsedAtPeer;
}

// set
void SkySegmentReference::setRefType(int refType) {
    SkySegmentReference::refType = refType;
}

void SkySegmentReference::setTraceId(const std::string &traceId) {
    SkySegmentReference::traceId = traceId;
}

void SkySegmentReference::setParentTraceSegmentId(const std::string &parentTraceSegmentId) {
    SkySegmentReference::parentTraceSegmentId = parentTraceSegmentId;
}

void SkySegmentReference::setParentSpanId(int parentSpanId) {
    SkySegmentReference::parentSpanId = parentSpanId;
}

void SkySegmentReference::setParentService(const std::string &parentService) {
    SkySegmentReference::parentService = parentService;
}

void SkySegmentReference::setParentServiceInstance(const std::string &parentServiceInstance) {
    SkySegmentReference::parentServiceInstance = parentServiceInstance;
}

void SkySegmentReference::setParentEndpoint(const std::string &parentEndpoint) {
    SkySegmentReference::parentEndpoint = parentEndpoint;
}

void SkySegmentReference::setNetworkAddressUsedAtPeer(const std::string &networkAddressUsedAtPeer) {
    SkySegmentReference::networkAddressUsedAtPeer = networkAddressUsedAtPeer;
}
