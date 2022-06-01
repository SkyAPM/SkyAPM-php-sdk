/*
 * Copyright 2022 SkyAPM
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

#ifndef SKYWALKING_SKY_CORE_REPORT_H
#define SKYWALKING_SKY_CORE_REPORT_H

typedef struct sky_core_report_t {
    char *address;
    char *service;
    char *service_instance;
} sky_core_report_t;

bool sky_core_report_ipc_init(size_t max_length);

bool sky_core_report_ipc_send(char *data, size_t len);

bool sky_core_report_new(char *address, char *service, char *service_instance, char *log_level, char *log_path);

char *sky_core_report_trace_id();

char *sky_core_service_instance_id();

#endif //SKYWALKING_SKY_CORE_REPORT_H
