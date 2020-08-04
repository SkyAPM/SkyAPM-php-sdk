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

#ifndef SKYWALKING_SEGMENT_WRAPPER_H
#define SKYWALKING_SEGMENT_WRAPPER_H

#define SPAN_TYPE_ENTRY 0
#define SPAN_TYPE_EXIT 1
#define SPAN_TYPE_LOCAL 2

#define SPAN_LAYER_HTTP 3

#ifdef __cplusplus
extern "C"
{
#endif

void *segment_init(char *serviceId, char *serviceInstanceId, int version, char *header);

void *segment_create_span(void *segment, int type);
char *segment_create_header(void *segment, void *span);
void segment_create_refs(void *segment);
char *segment_marshal(void *segment, int status_code);

void segment_free(void *segment);

#ifdef __cplusplus
}
#endif


#endif //SKYWALKING_SEGMENT_WRAPPER_H
