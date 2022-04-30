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
#ifndef SKYWALKING_LOGGING_DATA_H
#define SKYWALKING_LOGGING_DATA_H
#include <string>
#include <vector>
#include <src/logging/logging_data_body.h>
#include <src/logging/logging_trace_context.h>
#include <src/logging/logging_tag.h>

class LogData{
    private:
        long _timestamp;
        std::string _service;
        std::string _serviceInstance;
        std::string _endpoint;
        LogDataBody* _body;
        LogTraceContext* _traceContext;
        std::vector<LogTag *> _tags;
        std::string _layer;
    public:
        LogData();
        ~LogData();
        void setTimestamp(long timestamp);
        void setService(const std::string &service);
        void setServiceInstance(const std::string &serviceInstance);
        void setEndpoint(const std::string &endpoint);
        void addBody(const ContentType type, const std::string &content);
        void addTraceContext(const std::string &traceId, const std::string &traceSegmentId, const int spanId);
        void addTraceContext(const std::string &traceId);
        void addTag(const std::string &key, const std::string &value);
        void setLayer(const std::string &layer);
        std::string marshal();
};
#endif