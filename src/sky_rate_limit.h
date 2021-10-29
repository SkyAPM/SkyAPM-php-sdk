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


#ifndef SKYWALKING_SKY_RATE_LIMIT_H
#define SKYWALKING_SKY_RATE_LIMIT_H

#include <cstdint>
#include <chrono>
#include <atomic>

class FixedWindowRateLimiter {
public:
    using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

    FixedWindowRateLimiter(int64_t rate, int seconds = 3);
    bool validate();

private:
    std::atomic<std::int64_t> currentCount;
    int64_t rate;
    std::chrono::duration<int> timeWindow; // second
    TimePoint startTime;

    std::atomic_bool resetLock;
};

#endif // SKYWALKING_SKY_RATE_LIMIT_H
