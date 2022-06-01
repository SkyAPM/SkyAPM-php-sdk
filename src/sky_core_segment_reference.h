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



#ifndef SKYWALKING_SKY_CORE_SEGMENT_REFERENCE_H
#define SKYWALKING_SKY_CORE_SEGMENT_REFERENCE_H

typedef enum sky_core_segment_ref_type {
    CrossProcess, CrossThread
} sky_core_segment_ref_type;

typedef struct sky_core_segment_ref_t {
    // Tracing.proto
    sky_core_segment_ref_type refType;
    char *traceId;
    char *parentTraceSegmentId;
    int parentSpanId;
    char *parentService;
    char *parentServiceInstance;
    char *parentEndpoint;
    char *networkAddressUsedAtPeer;
} sky_core_segment_ref_t;

sky_core_segment_ref_t * sky_core_segment_ref_new();

#endif //SKYWALKING_SKY_CORE_SEGMENT_REFERENCE_H
