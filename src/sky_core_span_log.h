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

#include <string>
#include <vector>

class SkyCoreSpanLog {
public:
    SkyCoreSpanLog(std::string key, std::string value);

    long getTime();

    const std::string &getKey();

    const std::string &getValue();

private:
    long _time;
    std::string _key;
    std::string _value;
};

#endif //SKYWALKING_LOG_H
