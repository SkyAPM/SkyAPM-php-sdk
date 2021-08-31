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



#ifndef SKYWALKING_SEGMENT_H
#define SKYWALKING_SEGMENT_H

#define VERSION_8 8

#include <string>
#include <vector>
#include <src/network/v3/language-agent/Tracing.pb.h>
#include "cross_process_bag.h"
#include "span.h"

class Segment {
public:
    Segment(const std::string &serviceId, const std::string &serviceInstanceId, int version, const std::string &header);

    Span *createSpan(SkySpanType type, SkySpanLayer layer, int componentId);

    Span *findOrCreateSpan(const std::string &name, SkySpanType type, SkySpanLayer layer, int componentId);

    Span *firstSpan();

    std::string marshal();

    void setStatusCode(int code);

    std::string createHeader(Span *span);

    void createRefs();

    const std::string& getTraceId();

    void setSkip(bool skip);

    bool skip();

    ~Segment();

private:
    CrossProcessBag *bag;

    std::string _traceId;
    std::string _traceSegmentId;
    std::vector<Span *> spans;
    int _version;
    std::string _header;
    std::string _serviceId;
    std::string _serviceInstanceId;
    bool isSizeLimited = false;
    bool doSkip = false;

    int _status_code = 200;
};


#endif //SKYWALKING_SEGMENT_H
