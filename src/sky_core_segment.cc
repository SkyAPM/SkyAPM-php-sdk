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
#include "sky_core_cross_process.h"
#include "sky_go_wrapper.h"
#include "sky_go_utils.h"

SkyCoreSegment::SkyCoreSegment(const std::string &header) {

    SkyCoreSegment::service = "";
    SkyCoreSegment::serviceInstance = "";
    SkyCoreSegment::header = header;

    std::string segmentId(GenerateTraceId());
    SkyCoreSegment::traceSegmentId = segmentId;
    cp = new SkyCoreCrossProcess(header, segmentId);
    cp->setService(service);
    cp->setServiceInstance(serviceInstance);
    SkyCoreSegment::traceId = cp->getTraceId();
}

SkyCoreSegment::~SkyCoreSegment() {
    delete cp;

    for (auto span: spans) {
        delete span;
    }
    spans.clear();
    spans.shrink_to_fit();
}

std::string SkyCoreSegment::marshal() {
    this->isSkip;
    return "";
//    if (!spans.empty()) {
//        auto span = spans.front();
//        span->setEndTIme();
//        if (_status_code >= 400) {
//            span->setIsError(true);
//        }
//        span->pushTag(new SkyCoreTag("status_code", std::to_string(_status_code)));
//    }
//
//    SegmentObject msg;
//    msg.set_traceid(_traceId);
//    msg.set_tracesegmentid(_traceSegmentId);
//
//    for (auto span: spans) {
//        auto _span = msg.add_spans();
//        _span->set_spanid(span->getSpanId());
//        _span->set_parentspanid(span->getParentSpanId());
//        _span->set_starttime(span->getStartTime());
//        _span->set_endtime(span->getEndTime());
//
//        for (auto ref: span->getRefs()) {
//            auto _ref = _span->add_refs();
//            _ref->set_reftype(static_cast<RefType>(ref->getRefType()));
//            _ref->set_traceid(ref->getTraceId());
//            _ref->set_parenttracesegmentid(ref->getParentTraceSegmentId());
//            _ref->set_parentspanid(ref->getParentSpanId());
//            _ref->set_parentservice(ref->getParentService());
//            _ref->set_parentserviceinstance(ref->getParentServiceInstance());
//            _ref->set_parentendpoint(ref->getParentEndpoint());
//            _ref->set_networkaddressusedatpeer(ref->getNetworkAddressUsedAtPeer());
//        }
//
//        _span->set_operationname(span->getOperationName());
//        _span->set_peer(span->getPeer());
//        _span->set_spantype(static_cast<SpanType>(span->getSpanType()));
//        _span->set_spanlayer(static_cast<SpanLayer>(span->getSpanLayer()));
//        _span->set_componentid(span->getComponentId());
//        _span->set_iserror(span->getIsError());
//
//        for (auto tag: span->getTags()) {
//            auto _tag = _span->add_tags();
//            _tag->set_key(tag->getKey());
//            _tag->set_value(tag->getValue());
//        }
//
//        for (auto log: span->getLogs()) {
//            auto _log = _span->add_logs();
//            _log->set_time(log->getTime());
//            auto data = _log->add_data();
//            data->set_key(log->getKey());
//            data->set_value(log->getValue());
//        }
//
//        _span->set_skipanalysis(span->getSkipAnalysis());
//    }
//
//    msg.set_service(_serviceId);
//    msg.set_serviceinstance(_serviceInstanceId);
//    msg.set_issizelimited(false);
//    return msg.SerializeAsString();
}

void SkyCoreSegment::setStatusCode(int code) {
//    _status_code = code;
}

SkyCoreSpan *SkyCoreSegment::createSpan(SkyCoreSpanType type, SkyCoreSpanLayer layer, int componentId) {
    SkyCoreSpan *span = new SkyCoreSpan();
    span->setSpanType(type);
    span->setSpanLayer(layer);
    span->setComponentId(componentId);

    int id = 0;
    int parentId = -1;
    if (!spans.empty()) {
        id = spans.back()->getSpanId() + 1;
        parentId = 0;
    }

    span->setSpanId(id);
    span->setParentSpanId(parentId);


    spans.push_back(span);
    return span;
}

SkyCoreSpan *SkyCoreSegment::findOrCreateSpan(const std::string &name, SkyCoreSpanType type, SkyCoreSpanLayer layer,
                                              int componentId) {
    if (!spans.empty()) {
        for (auto item: spans) {
            if (item->getOperationName() == name) {
                return item;
            }
        }
    }
    auto span = createSpan(type, layer, componentId);
    span->setOperationName(name);
    return span;
}

SkyCoreSpan *SkyCoreSegment::firstSpan() {
    return spans.at(0);
}

std::string SkyCoreSegment::createHeader(SkyCoreSpan *span) {
    return cp->encode(span->getSpanId(), span->getPeer());
}

void SkyCoreSegment::createRefs() {
    if (!spans.empty()) {
        SkyCoreSpan *sp = spans.front();
        cp->setEndpoint(sp->getOperationName());

        if (!header.empty() && !cp->getParentTraceSegmentId().empty()) {
            auto ref = new SkyCoreSegmentReference();
            ref->setRefType(SkyCoreRefType::CrossProcess);
            ref->setTraceId(cp->getTraceId());
            ref->setParentTraceSegmentId(cp->getParentTraceSegmentId());
            ref->setParentSpanId(cp->getParentSpanId());
            ref->setParentService(cp->getParentService());
            ref->setParentServiceInstance(cp->getParentServiceInstance());
            ref->setParentEndpoint(cp->getParentEndpoint());
            ref->setNetworkAddressUsedAtPeer(cp->getNetworkAddress());
            sp->pushRefs(ref);
        }
    }
}

const std::string &SkyCoreSegment::getTraceId() {
    return traceId;
}

void SkyCoreSegment::setSkip(bool skip) {
    isSkip = skip;
}

bool SkyCoreSegment::skip() const {
    return isSkip;
}
