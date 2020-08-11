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

#ifndef SKYWALKING_COMMON_H
#define SKYWALKING_COMMON_H

#ifdef __cplusplus
#define SKY_BEGIN_EXTERN_C() extern "C" {
#define SKY_END_EXTERN_C() }
#else
#define SKY_BEGIN_EXTERN_C()
#define SKY_END_EXTERN_C()
#endif

struct service_info {
    char service[1024];
    char service_instance[1024];
};

#endif //SKYWALKING_COMMON_H
