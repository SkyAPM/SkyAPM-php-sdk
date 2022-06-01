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


#ifndef SKYWALKING_LOG_H
#define SKYWALKING_LOG_H

typedef struct sky_core_log_data_t {
    const char *key;
    const char *value;
} sky_core_log_data_t;

typedef struct sky_core_log_t {
    int data_total;
    int data_size;

    long time;
    sky_core_log_data_t **data;
} sky_core_log_t;

sky_core_log_t *sky_core_log_new();

void sky_core_log_add_data(sky_core_log_t *log, const char *key, const char *value);

void sky_core_log_free(sky_core_log_t *tag);

char *sky_core_log_to_json(sky_core_log_t *span);

//
//
//class SkyCoreLog {
//public:
//    SkyCoreLog(std::string key, std::string value);
//
//    long getTime();
//
//    const std::string &getKey();
//
//    const std::string &getValue();
//
//private:
//    long _time;
//    std::string _key;
//    std::string _value;
//};

#endif //SKYWALKING_LOG_H
