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

#include <string>
#include <vector>
#include "sky_core_tag.h"
#include "sky_core_segment_reference.h"
#include "sky_core_log.h"

enum class SkyCoreSpanType {
    Entry, Exit, Local
};

enum class SkyCoreSpanLayer {
    Unknown,
    Database,
    RPCFramework,
    Http,
    MQ,
    Cache,
    FAAS
};

class SkyCoreSpan {
public:
    SkyCoreSpan();

    ~SkyCoreSpan();

    int getSpanId() const;

    int getParentSpanId() const;

    long getStartTime() const;

    long getEndTime() const;

    const std::vector<SkyCoreSegmentReference *> &getRefs();

    const std::string &getOperationName();

    const std::string &getPeer();

    SkyCoreSpanType getSpanType();

    SkyCoreSpanLayer getSpanLayer();

    int getComponentId() const;

    bool getIsError() const;

    const std::vector<SkyCoreTag *> &getTags();

    const std::vector<SkyCoreLog *> &getLogs();

    bool getSkipAnalysis() const;

    // //
    void setEndTIme();

    void setOperationName(const std::string &name);

    void setPeer(const std::string &peer);

    void setSpanType(SkyCoreSpanType type);

    void setSpanLayer(SkyCoreSpanLayer layer);

    void setComponentId(int id);

    void setIsError(bool error);

    void setSpanId(int id);

    void setParentSpanId(int id);

    void pushTag(SkyCoreTag *tag);

    void pushLog(SkyCoreLog *log);

    void addTag(const std::string &key, const std::string &value);

    void addLog(const std::string &key, const std::string &value);

    void pushRefs(SkyCoreSegmentReference *ref);

private:
    // Tracing.proto
    int spanId;
    int parentSpanId;
    long startTime;
    long endTime;
    std::vector<SkyCoreSegmentReference *> refs;
    std::string operationName;
    std::string peer;
    SkyCoreSpanType spanType;
    SkyCoreSpanLayer spanLayer;
    int componentId;
    bool isError;
    std::vector<SkyCoreTag *> tags;
    std::vector<SkyCoreLog *> logs;
    bool skipAnalysis;
};


#endif //SKYWALKING_SKY_CORE_SPAN_H
