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
#include "sky_util_php.h"
#include "ext/standard/php_smart_string.h"

sky_core_tag_t *sky_core_tag_new(char *key, char *value) {
    sky_core_tag_t *tag = (sky_core_tag_t *) emalloc(sizeof(sky_core_tag_t));
    tag->key = (char *) emalloc(strlen(key) + 1);
    memcpy(tag->key, key, strlen(key) + 1);
    tag->value = (char *) emalloc(strlen(value) + 1);
    memcpy(tag->value, value, strlen(value) + 1);
    return tag;
}

int sky_core_tag_to_json(char **json, sky_core_tag_t *tag) {
    smart_string str = {0};
    smart_string_appendc(&str, '{');
    sky_util_json_str_ex(&str, "key", tag->key, strlen(tag->key));
    sky_util_json_str(&str, "value", tag->value, strlen(tag->value));
    smart_string_appendc(&str, '}');
    smart_string_0(&str);

    efree(tag->key);
    efree(tag->value);
    efree(tag);
    *json = str.c;
    return str.len;
}