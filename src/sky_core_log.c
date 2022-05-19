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


#include "sky_core_log.h"
#include "php.h"
#include <sys/time.h>
#include "sky_util_php.h"
#include "ext/standard/php_smart_string.h"

sky_core_log_t *sky_core_log_new() {
    sky_core_log_t *log = (sky_core_log_t *) emalloc(sizeof(sky_core_log_t));

    struct timeval tv;
    gettimeofday(&tv, NULL);
    log->time = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    log->data_total = 4;
    log->data_size = 0;
    log->data = (sky_core_log_data_t **) emalloc(log->data_total * sizeof(sky_core_log_data_t));
    return log;
}

void sky_core_log_add_data(sky_core_log_t *log, const char *key, const char *value) {
    if (log->data_size == log->data_total - 1) {
        sky_core_log_data_t **more = (sky_core_log_data_t **) erealloc(log->data, log->data_total * 2 * sizeof(sky_core_log_data_t));
        if (more != NULL) {
            log->data_total *= 2;
            log->data = more;
        } else {
            return;
        }
    }


    sky_core_log_data_t *data = (sky_core_log_data_t *) emalloc(sizeof(sky_core_log_data_t));
    data->key = key;
    data->value = value;
    log->data[log->data_size] = data;
    log->data_size++;
}

void sky_core_log_free(sky_core_log_t *log) {

}

char *sky_core_log_to_json(sky_core_log_t *log) {
    char *json;

    smart_string data = {0};
    smart_string_appendl(&data, "[", 1);

    for (int i = 0; i < log->data_size; ++i) {
        sky_core_log_data_t *log_data = log->data[i];
        char *data_json;
        asprintf(&data_json, "{\"key\":\"%s\",\"value\":\"%s\"}", log_data->key, log_data->value);
        smart_string_appendl(&data, data_json, strlen(data_json));
        free(data_json);
        if (i + 1 < log->data_size) {
            smart_string_appendl(&data, ",", 1);
        }
    }

    smart_string_appendl(&data, "]", 1);
    smart_string_0(&data);

    asprintf(&json, "{\"time\":%ld,\"data\":%s}", log->time, data.c);
    return json;
}
