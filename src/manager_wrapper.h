// Licensed to the Apache Software Foundation (ASF) under one or more
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

#ifndef SKYWALKING_MANAGER_WRAPPER_H
#define SKYWALKING_MANAGER_WRAPPER_H

struct service_info {
    char service[1024];
    char service_instance[1024];
};

#ifdef __cplusplus
extern "C"
{
#endif

void *manager_init(int version, char *code, char *grpc, struct service_info *info, int *fd);

#ifdef __cplusplus
}
#endif


#endif //SKYWALKING_MANAGER_WRAPPER_H
