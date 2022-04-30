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
#include <string>
#include <vector>
#include <src/logging/logging_data_body.h>

LogDataBody::LogDataBody(){}
LogDataBody::~LogDataBody(){}
LogDataBody::LogDataBody(const ContentType &type, const std::string &content):_type(type), _content(content){

}

ContentType LogDataBody::getType(){
    return this->_type;
}

std::string LogDataBody::getContent(){
    return this->_content;
}
