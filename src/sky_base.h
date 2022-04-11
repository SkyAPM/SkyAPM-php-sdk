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


#ifndef SKYWALKING_SKY_BASE_H
#define SKYWALKING_SKY_BASE_H

#include "pthread.h"

#ifdef __cplusplus
#define SKY_BEGIN_EXTERN_C() extern "C" {
#define SKY_END_EXTERN_C() }
#else
#define SKY_BEGIN_EXTERN_C()
#define SKY_END_EXTERN_C()
#endif

typedef struct {
    pthread_mutex_t lock;
    pthread_mutexattr_t lock_attr;
    char service[1024];
    char serviceInstance[1024];
    int id;
} mutex_service_t;

typedef mutex_service_t mutex_service;


#endif //SKYWALKING_SKY_BASE_H
