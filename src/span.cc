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

#include "span.h"

#include <ctime>
#include <chrono>
#include <iostream>

Span::Span() {
    startTime = getUnixTimeStamp();
    isError = false;
    skipAnalysis = false;
}

Span::~Span() {
    tags.clear();
    tags.shrink_to_fit();
}

// get
int Span::getSpanId() const {
    return spanId;
}

int Span::getParentSpanId() const {
    return parentSpanId;
}

long Span::getStartTime() const {
    return startTime;
}

long Span::getEndTime() const {
    return endTime;
}

std::vector<SkySegmentReference *> Span::getRefs() {
    return refs;
}

std::string Span::getOperationName() {
    return operationName;
}

std::string Span::getPeer() {
    return peer;
}

SkySpanType Span::getSpanType() {
    return spanType;
}

SkySpanLayer Span::getSpanLayer() {
    return spanLayer;
}

int Span::getComponentId() const {
    return componentId;
}

bool Span::getIsError() const {
    return isError;
}

std::vector<Tag *> Span::getTags() {
    return tags;
}

bool Span::getSkipAnalysis() const {
    return skipAnalysis;
}


// set
void Span::setEndTIme() {
    endTime = getUnixTimeStamp();
}

void Span::setOperationName(const std::string &name) {
    operationName = name.substr(0, name.find('?'));
}

void Span::setPeer(const std::string &p) {
    peer = p;
}

void Span::setSpanType(SkySpanType type) {
    spanType = type;
}

void Span::setSpanLayer(SkySpanLayer layer) {
    spanLayer = layer;
}

void Span::setComponentId(int id) {
    componentId = id;
}

void Span::setIsError(bool error) {
    isError = error;
}

void Span::setSpanId(int id) {
    spanId = id;
}

void Span::setParentSpanId(int id) {
    parentSpanId = id;
}

void Span::pushTag(Tag *tag) {
    tags.push_back(tag);
}

void Span::addTag(const std::string &key, const std::string &value) {
    tags.push_back(new Tag(key, value));
}

void Span::pushRefs(SkySegmentReference *ref) {
    refs.push_back(ref);
}

long Span::getUnixTimeStamp() {
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
    return ms.count();
}