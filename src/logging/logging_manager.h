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
#ifndef SKYWALKING_SKY_LOGGING_MANAGER_H
#define SKYWALKING_SKY_LOGGING_MANAGER_H
#include "src/manager.h"
class LogginManager{
    public:
    static void init(const ManagerOptions &options, struct service_info *info);
    static void cleanup();
    private: 
    LogginManager()=delete;
    static std::shared_ptr<grpc::ChannelCredentials> getCredentials(const ManagerOptions &options);
    static void logConsumer(const ManagerOptions &options, struct service_info *info);
};
#endif