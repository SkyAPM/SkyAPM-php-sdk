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

#ifndef SKYWALKING_SKY_SHM_H
#define SKYWALKING_SKY_SHM_H

#include "string"

#define IPC_KEY_PATH "/"
#define ACCESS_BIT 0666
#define SEM_EXIST (-2)
#define SEM_PROJ_ID '*'
#define SHM_PROJ_ID '-'
#define SHM_SIZE 0x400000

#if !defined(__APPLE__) && !defined(__MACH__)
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
} arg;
#endif

int sky_sem_new(int proj_id, int init_val);

int sky_sem_get(int proj_id);

int sky_sem_p(int sem_id);

int sky_sem_v(int sem_id);

int sky_sem_del(int sem_id);

int sky_shm_new(int proj_id, int size);

char *sky_shm_get_addr(int shm_id);

std::string sky_shm_read(char *shm_addr);

void sky_shm_write(char *shm_addr, char *buf);

int sky_shm_del(int shm_id);

void sky_shm_memset(char *shm_addr);

#endif //SKYWALKING_SKY_SHM_H
