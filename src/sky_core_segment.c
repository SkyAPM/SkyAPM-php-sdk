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

#include "sky_core_segment.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "php.h"
#include "sky_core_cross_process.h"
#include "sky_core_segment_reference.h"
#include "sky_go_wrapper.h"

sky_core_segment_t *sky_core_segment_new(char *protocol) {
    sky_core_segment_t *segment = (sky_core_segment_t *) emalloc(sizeof(sky_core_segment_t));
    segment->span_total = 64;
    segment->span_size = 0;

    // Tracing.proto
    segment->traceId = NULL;
    segment->traceSegmentId = skywalking_trace_id_new();
    segment->spans = (sky_core_span_t **) emalloc(segment->span_total);
    segment->service = (char *) emalloc(sizeof(char));
    memset(segment->service, 0, strlen(segment->service));
    segment->serviceInstance = (char *) emalloc(sizeof(char));
    memset(segment->serviceInstance, 0, strlen(segment->serviceInstance));


    segment->cross_process = sky_core_cross_process_new(protocol);
    sky_core_cross_process_set_trace_id(segment->cross_process, segment->traceSegmentId);
    segment->traceId = segment->cross_process->traceId;

    return segment;
}

void sky_core_segment_add_span(sky_core_segment_t *segment, sky_core_span_t *span) {
    if (segment->span_size == 0 && segment->cross_process->parentSegmentId != NULL) {
        sky_core_segment_ref_t *ref = sky_core_segment_ref_new();
        // todo
        sky_core_span_add_refs(span, ref);
    }


    if (segment->span_size == segment->span_total - 1) {
        sky_core_span_t **more = (sky_core_span_t **) erealloc(segment->spans, segment->span_total * 2);
        if (more != NULL) {
            segment->span_total *= 2;
            segment->spans = more;
        } else {
            return;
        }
    }

    if (segment->span_size != 0) {
        span->spanId = segment->spans[segment->span_size - 1]->spanId + 1;
        span->parentSpanId = 0;
    } else {
        span->spanId = 0;
        span->parentSpanId = -1;
    }

    segment->spans[segment->span_size] = span;
    segment->span_size++;
}

char *sky_core_segment_to_json(sky_core_segment_t *segment) {
    char *json;
    char *temp = "{\"traceId\":\"%s\",\"traceSegmentId\":\"%s\",\"spans\":%s,\"service\":\"%s\",\"serviceInstance\":\"%s\",\"isSizeLimited\":%s}";

    asprintf(&json, temp,
             segment->traceId,
             segment->traceSegmentId,
             "[{}]",
             segment->service,
             segment->serviceInstance,
             btoa(segment->isSizeLimited)
    );
    return json;
}

//SkyCoreSegment::SkyCoreSegment(const std::string &header) {
//
//    SkyCoreSegment::service = "";
//    SkyCoreSegment::serviceInstance = "";
//    SkyCoreSegment::header = header;
//
//    std::string segmentId(GenerateTraceId());
//    SkyCoreSegment::traceSegmentId = segmentId;
//    cp = new SkyCoreCrossProcess(header, segmentId);
//    cp->setService(service);
//    cp->setServiceInstance(serviceInstance);
//    SkyCoreSegment::traceId = cp->getTraceId();
//}
//
//SkyCoreSegment::~SkyCoreSegment() {
//    delete cp;
//
//    for (auto span: spans) {
//        delete span;
//    }
//    spans.clear();
//    spans.shrink_to_fit();
//}
//
//std::string SkyCoreSegment::marshal() {
//    nlohmann::json j;
//    j["traceId"] = traceId;
//    j["traceSegmentId"] = traceSegmentId;
//    j["spans"] = nlohmann::json::array();
//    for (auto span: spans) {
//        j["spans"].push_back(nlohmann::json::parse(span->marshal()));
//    }
//    j["service"] = service;
//    j["serviceInstance"] = serviceInstance;
//    j["isSizeLimited"] = isSizeLimited;
//    return j.dump();
////    if (!spans.empty()) {
////        auto span = spans.front();
////        span->setEndTIme();
////        if (_status_code >= 400) {
////            span->setIsError(true);
////        }
////        span->pushTag(new SkyCoreTag("status_code", std::to_string(_status_code)));
////    }
////
////    SegmentObject msg;
////    msg.set_traceid(_traceId);
////    msg.set_tracesegmentid(_traceSegmentId);
////
////    for (auto span: spans) {
////        auto _span = msg.add_spans();
////        _span->set_spanid(span->getSpanId());
////        _span->set_parentspanid(span->getParentSpanId());
////        _span->set_starttime(span->getStartTime());
////        _span->set_endtime(span->getEndTime());
////
////        for (auto ref: span->getRefs()) {
////            auto _ref = _span->add_refs();
////            _ref->set_reftype(static_cast<RefType>(ref->getRefType()));
////            _ref->set_traceid(ref->getTraceId());
////            _ref->set_parenttracesegmentid(ref->getParentTraceSegmentId());
////            _ref->set_parentspanid(ref->getParentSpanId());
////            _ref->set_parentservice(ref->getParentService());
////            _ref->set_parentserviceinstance(ref->getParentServiceInstance());
////            _ref->set_parentendpoint(ref->getParentEndpoint());
////            _ref->set_networkaddressusedatpeer(ref->getNetworkAddressUsedAtPeer());
////        }
////
////        _span->set_operationname(span->getOperationName());
////        _span->set_peer(span->getPeer());
////        _span->set_spantype(static_cast<SpanType>(span->getSpanType()));
////        _span->set_spanlayer(static_cast<SpanLayer>(span->getSpanLayer()));
////        _span->set_componentid(span->getComponentId());
////        _span->set_iserror(span->getIsError());
////
////        for (auto tag: span->getTags()) {
////            auto _tag = _span->add_tags();
////            _tag->set_key(tag->getKey());
////            _tag->set_value(tag->getValue());
////        }
////
////        for (auto log: span->getLogs()) {
////            auto _log = _span->add_logs();
////            _log->set_time(log->getTime());
////            auto data = _log->add_data();
////            data->set_key(log->getKey());
////            data->set_value(log->getValue());
////        }
////
////        _span->set_skipanalysis(span->getSkipAnalysis());
////    }
////
////    msg.set_service(_serviceId);
////    msg.set_serviceinstance(_serviceInstanceId);
////    msg.set_issizelimited(false);
////    return msg.SerializeAsString();
//}
//
//void SkyCoreSegment::setStatusCode(int code) {
////    _status_code = code;
//}
//
//SkyCoreSpan *SkyCoreSegment::createSpan(SkyCoreSpanType type, SkyCoreSpanLayer layer, int componentId) {
//    SkyCoreSpan *span = new SkyCoreSpan();
//    span->setSpanType(type);
//    span->setSpanLayer(layer);
//    span->setComponentId(componentId);
//
//    int id = 0;
//    int parentId = -1;
//    if (!spans.empty()) {
//        id = spans.back()->getSpanId() + 1;
//        parentId = 0;
//    }
//
//    span->setSpanId(id);
//    span->setParentSpanId(parentId);
//
//
//    spans.push_back(span);
//    return span;
//}
//
//SkyCoreSpan *SkyCoreSegment::findOrCreateSpan(const std::string &name, SkyCoreSpanType type, SkyCoreSpanLayer layer,
//                                              int componentId) {
//    if (!spans.empty()) {
//        for (auto item: spans) {
//            if (item->getOperationName() == name) {
//                return item;
//            }
//        }
//    }
//    auto span = createSpan(type, layer, componentId);
//    span->setOperationName(name);
//    return span;
//}
//
//SkyCoreSpan *SkyCoreSegment::firstSpan() {
//    return spans.at(0);
//}
//
//std::string SkyCoreSegment::createHeader(SkyCoreSpan *span) {
//    return cp->encode(span->getSpanId(), span->getPeer());
//}
//
//void SkyCoreSegment::createRefs() {
//    if (!spans.empty()) {
//        SkyCoreSpan *sp = spans.front();
//        cp->setEndpoint(sp->getOperationName());
//
//        if (!header.empty() && !cp->getParentTraceSegmentId().empty()) {
//            auto ref = new SkyCoreSegmentReference();
//            ref->setRefType(SkyCoreRefType::CrossProcess);
//            ref->setTraceId(cp->getTraceId());
//            ref->setParentTraceSegmentId(cp->getParentTraceSegmentId());
//            ref->setParentSpanId(cp->getParentSpanId());
//            ref->setParentService(cp->getParentService());
//            ref->setParentServiceInstance(cp->getParentServiceInstance());
//            ref->setParentEndpoint(cp->getParentEndpoint());
//            ref->setNetworkAddressUsedAtPeer(cp->getNetworkAddress());
//            sp->pushRefs(ref);
//        }
//    }
//}
//
//const std::string &SkyCoreSegment::getTraceId() {
//    return traceId;
//}
//
//void SkyCoreSegment::setSkip(bool skip) {
//    isSkip = skip;
//}
//
//bool SkyCoreSegment::skip() const {
//    return isSkip;
//}