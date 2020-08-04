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

#include "segment_wrapper.h"
#include "segment.h"

void *segment_init(char *serviceId, char *serviceInstanceId, int version, char *header) {
    return new Segment(serviceId, serviceInstanceId, version, header);
}

void *segment_create_span(void *segment, int type) {
    return static_cast<Segment *>(segment)->createSpan(static_cast<SkySpanType>(type));
}

char *segment_create_header(void *segment, void *span) {
    std::string header = static_cast<Segment *>(segment)->createHeader(static_cast<Span *>(span));

    char *str = new char[header.length() + 1];
    strcpy(str, header.c_str());
    return str;
}

void segment_create_refs(void *segment) {
    return static_cast<Segment *>(segment)->createRefs();
}

char *segment_marshal(void *segment, int status_code) {
    std::string msg = static_cast<Segment *>(segment)->marshal(status_code);

    char *str = new char[msg.length() + 1];
    strcpy(str, msg.c_str());
    return str;
}

void segment_free(void *segment) {
    delete static_cast<Segment *>(segment);
}
