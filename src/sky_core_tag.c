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


#include "sky_core_tag.h"
#include <stdlib.h>
#include <stdio.h>

sky_core_tag_t *sky_core_tag_new(const char *key, const char *value) {
    sky_core_tag_t *tag = (sky_core_tag_t *) malloc(sizeof(sky_core_tag_t));
    tag->key = key;
    tag->value = value;
    return tag;
}

void sky_core_tag_free(sky_core_tag_t *tag) {

}

char *sky_core_tag_to_json(sky_core_tag_t *tag) {
    char *json;
    asprintf(&json, "{\"key\":\"%s\",\"value\":\"%s\"}", tag->key, tag->value);
    return json;
}