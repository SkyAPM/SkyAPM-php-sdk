// Licensed to the Apache Software Foundation (ASF) under one or more
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

#include "segment.h"
#include "manager.h"
#include "cross_process_bag.h"
#include "language-agent/Tracing.grpc.pb.h"

Segment::Segment(const std::string &serviceId, const std::string &serviceInstanceId, int version,
                 const std::string &header) {

    _serviceId = serviceId;
    _serviceInstanceId = serviceInstanceId;
    _header = header;
    _version = version;

    std::string traceId = Manager::generateUUID();
    traceId.erase(std::remove(traceId.begin(), traceId.end(), '-'), traceId.end());

    Segment::_traceId = traceId;
    Segment::_traceSegmentId = traceId;
    bag = new CrossProcessBag(_serviceId, _serviceInstanceId, traceId, _version, _header);

    if (!bag->getTraceId().empty()) {
        Segment::_traceId = bag->getTraceId();
    }
}

Segment::~Segment() {
    delete bag;
    spans.clear();
    spans.shrink_to_fit();
}

std::string Segment::marshal(int status_code) {
    if (!spans.empty()) {
        auto span = spans.front();
        span->setEndTIme();
        if (status_code >= 400) {
            span->setIsError(true);
        }
        span->pushTag(new Tag("status_code", std::to_string(status_code)));
    }

    SegmentObject msg;
    msg.set_traceid(_traceId);
    msg.set_tracesegmentid(_traceSegmentId);

    for (auto span: spans) {
        auto _span = msg.add_spans();
        _span->set_spanid(span->getSpanId());
        _span->set_parentspanid(span->getParentSpanId());
        _span->set_starttime(span->getStartTime());
        _span->set_endtime(span->getEndTime());

        for (auto ref:span->getRefs()) {
            auto _ref = _span->add_refs();
            _ref->set_reftype(static_cast<RefType>(ref->getRefType()));
            _ref->set_traceid(ref->getTraceId());
            _ref->set_parenttracesegmentid(ref->getParentTraceSegmentId());
            _ref->set_parentspanid(ref->getParentSpanId());
            _ref->set_parentservice(ref->getParentService());
            _ref->set_parentserviceinstance(ref->getParentServiceInstance());
            _ref->set_parentendpoint(ref->getParentEndpoint());
            _ref->set_networkaddressusedatpeer(ref->getNetworkAddressUsedAtPeer());
        }

        _span->set_operationname(span->getOperationName());
        _span->set_peer(span->getPeer());
        _span->set_spantype(static_cast<SpanType>(span->getSpanType()));
        _span->set_spanlayer(static_cast<SpanLayer>(span->getSpanLayer()));
        _span->set_componentid(span->getComponentId());
        _span->set_iserror(span->getIsError());

        for (auto tag:span->getTags()) {
            auto _tag = _span->add_tags();
            _tag->set_key(tag->getKey());
            _tag->set_value(tag->getValue());
        }

        // todo logs
        _span->set_skipanalysis(span->getSkipAnalysis());
    }

    msg.set_service(_serviceId);
    msg.set_serviceinstance(_serviceInstanceId);
    msg.set_issizelimited(false);
    return msg.SerializeAsString();
}

Span *Segment::createSpan(SkySpanType type, SkySpanLayer layer, int componentId) {
    Span *span = new Span();
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

std::string Segment::createHeader(Span *span) {
    return bag->encode(span->getSpanId(), span->getPeer());
}

void Segment::createRefs() {
    if (!spans.empty()) {
        Span *sp = spans.front();
        bag->setOperationName(sp->getOperationName());

        if (!_header.empty() && !bag->getParentTraceSegmentId().empty()) {
            auto ref = new SkySegmentReference();
            ref->setRefType(0);
            ref->setTraceId(bag->getTraceId());
            ref->setParentTraceSegmentId(bag->getParentTraceSegmentId());
            ref->setParentSpanId(bag->getParentSpanId());
            ref->setParentService(bag->getParentService());
            ref->setParentServiceInstance(bag->getParentServiceInstance());
            ref->setParentEndpoint(bag->getParentEndpoint());
            ref->setNetworkAddressUsedAtPeer(bag->getNetworkAddressUsedAtPeer());
            sp->pushRefs(ref);
        }
    }
}

std::string Segment::getTraceId() {
    return _traceId;
}