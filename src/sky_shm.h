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


#ifndef SKYWALKING_SKY_SHM_H
#define SKYWALKING_SKY_SHM_H

#include "string"

#define ACCESS_BIT 0666
#define SEM_EXIST (-2)

#if !defined(__APPLE__) && !defined(__MACH__)
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};
#endif

int sky_sem_new();

int sky_sem_get();

int sky_sem_p(int sem_id);

int sky_sem_v(int sem_id);

int sky_sem_del(int sem_id);


#endif //SKYWALKING_SKY_SHM_H
