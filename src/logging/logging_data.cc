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
#include <src/logging/logging_data.h>
#include "logging/Logging.grpc.pb.h"

LogData::LogData(){
    this->_traceContext = nullptr;
    this->_body = nullptr;

};
LogData::~LogData(){
    if (this->_traceContext) {
        delete this->_traceContext;
    }

    if (this->_body) {
        delete this->_body;
    }

    if (!this->_tags.empty()) {
        for(auto tag : this->_tags) {
            delete tag;
        }

        this->_tags.clear();
        this->_tags.shrink_to_fit();
    }
};

void LogData::setTimestamp(long timestamp){
    this->_timestamp = timestamp;
}
void LogData::setService(const std::string &service) {
    this->_service = service;
}
void LogData::setServiceInstance(const std::string &serviceInstance) {
    this->_serviceInstance = serviceInstance;
}
void LogData::setEndpoint(const std::string &endpoint) {
    this->_endpoint = endpoint;
}
void LogData::addBody(const ContentType type, const std::string &content) {
    this->_body = new LogDataBody(type, content);
}
void LogData::addTraceContext(const std::string &traceId, const std::string &traceSegmentId, const int spanId) {
    this->_traceContext = new LogTraceContext(traceId, traceSegmentId, spanId);
}
void LogData::addTraceContext(const std::string &traceId) {
    this->_traceContext = new LogTraceContext(traceId, traceId, 0);
}
void LogData::addTag(const std::string &key, const std::string &value) {
    this->_tags.push_back(new LogTag(key, value));
}
void LogData::setLayer(const std::string &layer) {
    this->_layer = layer;
}
std::string LogData::marshal() {
    skywalking::v3::LogData logData;
    logData.set_service(this->_service);
    logData.set_serviceinstance(this->_serviceInstance);
    logData.set_timestamp(this->_timestamp);
    logData.set_layer(this->_layer);

    if (this->_traceContext) {
        skywalking::v3::TraceContext *traceContext = logData.mutable_tracecontext();
        traceContext->set_spanid(this->_traceContext->getSpanId());
        traceContext->set_traceid(this->_traceContext->getTraceId());
        traceContext->set_tracesegmentid(this->_traceContext->getTraceSegmentId());
    }
   
    if (this->_body) {
        skywalking::v3::LogDataBody *logDataBody = logData.mutable_body();
        if (this->_body->getType() == TEXT) {
            logDataBody->set_type("TEXT");
            skywalking::v3::TextLog *textLog = logDataBody->mutable_text();
            textLog->set_text(this->_body->getContent());
        } else if (this->_body->getType() == JSON) {
            logDataBody->set_type("JSON");
            skywalking::v3::JSONLog *jsonLog = logDataBody->mutable_json();
            jsonLog->set_json(this->_body->getContent());
        } else if (this->_body->getType() == YAML) {
            logDataBody->set_type("YAML");
            skywalking::v3::YAMLLog *yamlLog = logDataBody->mutable_yaml();
            yamlLog->set_yaml(this->_body->getContent());
        }
    }
    
    if (!this->_tags.empty()) {
        skywalking::v3::LogTags *logTags = logData.mutable_tags();
        for(auto tag : this->_tags) {
            KeyStringValuePair *pair = logTags->add_data();
            pair->set_key(tag->getKey());
            pair->set_value(tag->getValue());
        }
    }    
    return logData.SerializeAsString();
}