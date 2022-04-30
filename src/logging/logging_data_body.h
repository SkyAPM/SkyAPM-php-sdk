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
#ifndef SKYWALKING_LOGGING_DATA_BODY_H
#define SKYWALKING_LOGGING_DATA_BODY_H
#include <string>
#include <vector>


enum ContentType {
    TEXT,
    JSON,
    YAML
};

class LogDataBody{
    private:
        ContentType _type;
        std::string _content;
    public:
        LogDataBody();
        LogDataBody(const ContentType &type, const std::string &content);
        ~LogDataBody();
        ContentType getType();
        std::string getContent();
};
#endif