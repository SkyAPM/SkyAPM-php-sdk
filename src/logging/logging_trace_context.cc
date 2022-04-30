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
#include <string>
#include <src/logging/logging_trace_context.h>

LogTraceContext::LogTraceContext(){};
LogTraceContext::~LogTraceContext(){};
LogTraceContext::LogTraceContext(const std::string &traceId, const std::string &traceSegmentId, const int spanId)
:_traceId(traceId),_traceSegmentId(traceSegmentId),_spanId(spanId)
{};

std::string LogTraceContext::getTraceId() {
    return this->_traceId;
};
std::string LogTraceContext::getTraceSegmentId() {
    return this->_traceSegmentId;
}
int LogTraceContext::getSpanId() {
    return this->_spanId;
}