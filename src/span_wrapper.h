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

#ifndef SKYWALKING_SPAN_WRAPPER_H
#define SKYWALKING_SPAN_WRAPPER_H

#ifdef __cplusplus
extern "C"
{
#endif

void *span_init();

void span_set_end_time(void *span);

void span_set_operation_name(void *span, char *name);

void span_set_peer(void *span, char *peer);

void span_set_span_layer(void *span, int layer);

void span_set_component_id(void *span, int id);

void span_set_is_error(void *span, int error);

void span_put_tag(void *span, void *tag);

void span_free(void *span);

#ifdef __cplusplus
}
#endif

#endif //SKYWALKING_SPAN_WRAPPER_H
