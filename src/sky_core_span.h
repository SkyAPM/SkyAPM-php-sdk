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


#ifndef SKYWALKING_SKY_CORE_SPAN_H
#define SKYWALKING_SKY_CORE_SPAN_H

#include <stdbool.h>
#include "sky_core_tag.h"
#include "sky_core_log.h"
#include "sky_core_segment_reference.h"

typedef enum sky_core_span_type {
    Entry, Exit, Local
} sky_core_span_type;

typedef enum sky_core_span_layer {
    Unknown,
    Database,
    RPCFramework,
    Http,
    MQ,
    Cache,
    FAAS
} sky_core_span_layer;

typedef struct sky_core_span_t {
    int refs_total;
    int refs_size;

    int tags_total;
    int tags_size;

    int logs_total;
    int logs_size;

    // Tracing.proto
    int spanId;
    int parentSpanId;
    long startTime;
    long endTime;
    sky_core_segment_ref_t **refs;
    char *operationName;
    char *peer;
    sky_core_span_type spanType;
    sky_core_span_layer spanLayer;
    int componentId;
    bool isError;
    sky_core_tag_t **tags;
    sky_core_log_t **logs;
    bool skipAnalysis;
} sky_core_span_t;

sky_core_span_t *sky_core_span_new(sky_core_span_type type, sky_core_span_layer layer, int componentId);

void sky_core_span_set_end_time(sky_core_span_t *span);

void sky_core_span_add_refs(sky_core_span_t *span, sky_core_segment_ref_t *ref);

void sky_core_span_set_operation_name(sky_core_span_t *span, char *name);

void sky_core_span_set_peer(sky_core_span_t *span, char *peer);

void sky_core_span_set_error(sky_core_span_t *span, bool isError);

void sky_core_span_add_tag(sky_core_span_t *span, sky_core_tag_t *tag);

int sky_core_span_to_json(char **json, sky_core_span_t *span);

//class SkyCoreSpan {
//public:
//    SkyCoreSpan();
//
//    ~SkyCoreSpan();
//
//    int getSpanId() const;
//
//    int getParentSpanId() const;
//
//    long getStartTime() const;
//
//    long getEndTime() const;
//
//    const std::vector<SkyCoreSegmentReference *> &getRefs();
//
//    const std::string &getOperationName();
//
//    const std::string &getPeer();
//
//    SkyCoreSpanType getSpanType();
//
//    SkyCoreSpanLayer getSpanLayer();
//
//    int getComponentId() const;
//
//    bool getIsError() const;
//
//    const std::vector<SkyCoreTag *> &getTags();
//
//    const std::vector<SkyCoreLog *> &getLogs();
//
//    bool getSkipAnalysis() const;
//
//    // SET
//    void setSpanId(int id);
//
//    void setParentSpanId(int id);
//
//    void setEndTIme();
//
//    void pushRefs(SkyCoreSegmentReference *ref);
//
//    void setOperationName(const std::string &name);
//
//    void setPeer(const std::string &peer);
//
//    void setSpanType(SkyCoreSpanType type);
//
//    void setSpanLayer(SkyCoreSpanLayer layer);
//
//    void setComponentId(int id);
//
//    void setIsError(bool error);
//
//    void pushTag(SkyCoreTag *tag);
//
//    void pushLog(SkyCoreLog *log);
//
//    void addTag(const std::string &key, const std::string &value);
//
//    void addLog(const std::string &key, const std::string &value);
//
//    std::string marshal();
//
//private:
//    // Tracing.proto
//    int spanId;
//    int parentSpanId;
//    long startTime;
//    long endTime;
//    std::vector<SkyCoreSegmentReference *> refs;
//    std::string operationName;
//    std::string peer;
//    SkyCoreSpanType spanType;
//    SkyCoreSpanLayer spanLayer;
//    int componentId;
//    bool isError;
//    std::vector<SkyCoreTag *> tags;
//    std::vector<SkyCoreLog *> logs;
//    bool skipAnalysis;
//};


#endif //SKYWALKING_SKY_CORE_SPAN_H
