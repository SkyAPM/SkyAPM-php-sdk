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


#include "sky_shm.h"

#include "sys/ipc.h"
#include "sys/sem.h"
#include <cerrno>

int sky_sem_new() {
    int sem_id;
    union semun sem_union;

    sem_id = semget(IPC_PRIVATE, 1, ACCESS_BIT | IPC_CREAT | IPC_EXCL);

    if (sem_id < 0) {
        if (errno == EEXIST) {
            return SEM_EXIST;
        }
        return -1;
    }

    sem_union.val = 1;
    if ((semctl(sem_id, 0, SETVAL, sem_union)) < 0) {
        return -1;
    }
    return sem_id;
}

int sky_sem_get() {
    int sem_id = semget(IPC_PRIVATE, 1, ACCESS_BIT);
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