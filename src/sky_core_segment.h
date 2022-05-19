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



#ifndef SKYWALKING_SKY_CORE_SEGMENT_H
#define SKYWALKING_SKY_CORE_SEGMENT_H

#include <stdbool.h>
#include "sky_core_span.h"
#include "sky_core_cross_process.h"

#define btoa(x) ((x)?"true":"false")

typedef struct sky_core_segment_t {
    bool isSkip;
    sky_core_cross_process_t *cross_process;

    int span_total;
    int span_size;

    // Tracing.proto
    char *traceId;
    char *traceSegmentId;
    sky_core_span_t **spans;
    char *service;
    char *serviceInstance;
    bool isSizeLimited;
} sky_core_segment_t;

sky_core_segment_t *sky_core_segment_new(char *protocol);

void sky_core_segment_add_span(sky_core_segment_t *segment, sky_core_span_t *span);

void sky_core_segment_set_service(sky_core_segment_t *segment, char *service);

void sky_core_segment_set_service_instance(sky_core_segment_t *segment, char *instance);

int sky_core_segment_to_json(char **json, sky_core_segment_t *segment);

//#include <string>
//#include <vector>
//#include "sky_core_cross_process.h"
//#include "sky_core_span.h"
//
//class SkyCoreSegment{
//        public:
//        explicit SkyCoreSegment(const std::string &header);
//
//        SkyCoreSpan *createSpan(SkyCoreSpanType type, SkyCoreSpanLayer layer, int componentId);
//
//        SkyCoreSpan *
//        findOrCreateSpan(const std::string &name, SkyCoreSpanType type, SkyCoreSpanLayer layer, int componentId);
//
//        SkyCoreSpan *firstSpan();
//
//        std::string marshal();
//
//        void setStatusCode(int code);
//
//        std::string createHeader(SkyCoreSpan *span);
//
//        void createRefs();
//
//        const std::string &getTraceId();
//
//        void setSkip(bool skip);
//
//        bool skip() const;
//
//        ~SkyCoreSegment();
//
//        private:
//        SkyCoreCrossProcess *cp;
//        std::string header;
//        bool isSkip;
//
//        // Tracing.proto
//        std::string traceId;
//        std::string traceSegmentId;
//        std::vector<SkyCoreSpan *> spans;
//        std::string service;
//        std::string serviceInstance;
//        bool isSizeLimited = false;
//};


#endif //SKYWALKING_SKY_CORE_SEGMENT_H
