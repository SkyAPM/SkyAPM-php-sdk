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
#ifndef SKYWALKING_LOGGING_TRACE_CONTEXT_H
#define SKYWALKING_LOGGING_TRACE_CONTEXT_H
#include <string>
class LogTraceContext{
    private:
        std::string _traceId;
        std::string _traceSegmentId;
        int _spanId;
    public:
    LogTraceContext();
    LogTraceContext(const std::string &traceId, const std::string &traceSegmentId, const int spanId);
    ~LogTraceContext();
    std::string getTraceId();
    std::string getTraceSegmentId();
    int getSpanId();
};
#endif