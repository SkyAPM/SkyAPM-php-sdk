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

#include "span_wrapper.h"
#include "span.h"

void *span_init() {
    return new Span();
}

void span_set_end_time(void *span) {
    static_cast<Span *>(span)->setEndTIme();
}

void span_set_operation_name(void *span, char *name) {
    static_cast<Span *>(span)->setOperationName(name);
}

void span_set_peer(void *span, char *peer) {
    static_cast<Span *>(span)->setPeer(peer);
}

void span_set_span_layer(void *span, int layer) {
    static_cast<Span *>(span)->setSpanLayer(static_cast<SkySpanLayer>(layer));
}

void span_set_component_id(void *span, int id) {
    static_cast<Span *>(span)->setComponentId(id);
}

void span_set_is_error(void *span, int error) {
    static_cast<Span *>(span)->setIsError(error == 1);
}

void span_put_tag(void *span, void *tag) {
    static_cast<Span *>(span)->pushTag(static_cast<Tag *>(tag));
}

void span_free(void *span) {
    delete static_cast<Span *>(span);
}