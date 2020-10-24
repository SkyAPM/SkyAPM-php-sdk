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

#include "sky_shm.h"

#include "sys/ipc.h"
#include "sys/sem.h"
#include "sys/shm.h"
#include <cstring>
#include <cerrno>
#include <string>

int sky_sem_new(int proj_id, int init_val) {
    key_t key;
    int sem_id;
    union semun sem_union;

    key = ftok(IPC_KEY_PATH, proj_id);

    if (key < 0) {
        return -1;
    }

    sem_id = semget(key, 1, ACCESS_BIT | IPC_CREAT | IPC_EXCL);

    if (sem_id < 0) {
        if (errno == EEXIST) {
            return SEM_EXIST;
        }
        return -1;
    }

    sem_union.val = init_val;
    if ((semctl(sem_id, 0, SETVAL, sem_union)) < 0) {
        return -1;
    }
    return sem_id;
}

int sky_sem_get(int proj_id) {
    key_t key;
    int sem_id;

    key = ftok(IPC_KEY_PATH, proj_id);

    if (key < 0) {
        return -1;
    }

    sem_id = semget(key, 1, ACCESS_BIT);
    if (sem_id < 0) {
        return -1;
    }

    return sem_id;
}

int sky_sem_p(int sem_id) {

    struct sembuf sem_buf{};
    sem_buf.sem_num = 0;
    sem_buf.sem_op = -1;
    sem_buf.sem_flg = SEM_UNDO;

    if (semop(sem_id, &sem_buf, 1) < 0) {
        return -1;
    }

    return 0;
}

int sky_sem_v(int sem_id) {

    struct sembuf sem_buf{};
    sem_buf.sem_num = 0;
    sem_buf.sem_op = 1;
    sem_buf.sem_flg = SEM_UNDO;

    if (semop(sem_id, &sem_buf, 1) < 0) {
        return -1;
    }

    return 0;
}

int sky_sem_del(int sem_id) {
    union semun sem_union;
    if (semctl(sem_id, 0, IPC_RMID, sem_union) < 0) {
        return -1;
    }
    return 0;
}

int sky_shm_new(int proj_id, int size) {
    int shm_id;
    key_t key;
    key = ftok(IPC_KEY_PATH, proj_id);

    if (key < 0) {
        return -1;
    }

    shm_id = shmget(key, size, ACCESS_BIT | IPC_CREAT);

    if (shm_id < 0) {
        return -1;
    }

    return shm_id;
}

char *sky_shm_get_addr(int shm_id) {
    char *p;

    if ((p = static_cast<char *>(shmat(shm_id, nullptr, 0))) == (char *) -1) {
        return nullptr;
    }

    return p;
}

std::string sky_shm_read(char *shm_addr) {

    int len = strlen(shm_addr) + 1;
    char *buf = new char[len];
    strncpy(buf, shm_addr, strlen(shm_addr) + 1);
    std::string full(buf);
    delete[] buf;

    return full;
}

void sky_shm_write(char *shm_addr, char *buf) {
    strncpy(shm_addr, buf, strlen(buf) + 1);
}

int sky_shm_del(int shm_id) {
    if (shmctl(shm_id, IPC_RMID, nullptr) < 0) {
        return -1;
    }
    return 0;
}

void sky_shm_memset(char *shm_addr) {
    memset(shm_addr, 0, SHM_SIZE);
}