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


#ifndef SKYWALKING_SKY_CORE_TAG_H
#define SKYWALKING_SKY_CORE_TAG_H

typedef struct sky_core_tag_t {
    char *key;
    char *value;
} sky_core_tag_t;

sky_core_tag_t *sky_core_tag_new(char *key, char *value);

int sky_core_tag_to_json(char **json, sky_core_tag_t *tag);

#endif //SKYWALKING_SKY_CORE_TAG_H
