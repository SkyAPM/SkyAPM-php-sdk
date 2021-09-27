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



#include <cmath>
#include <cstdio>
#include "sky_rate_limit.h"
#include "sky_log.h"

FixedWindowRateLimiter::FixedWindowRateLimiter(int64_t rate, int seconds) : rate(rate), currentCount(0), resetLock(false) {
    if (seconds < 1) {
        timeWindow = std::chrono::seconds(1);
    } else {
        timeWindow = std::chrono::seconds(seconds);
    }

    this->startTime = TimePoint::clock::now();
}

bool FixedWindowRateLimiter::validate() {
    if (this->rate < 1) {
        return true;
    }

    std::chrono::duration<double> span = TimePoint::clock::now() - this->startTime;

    bool falseValue = false;
    if (span > this->timeWindow 
        && resetLock.compare_exchange_weak(falseValue, true) 
        && (TimePoint::clock::now() - this->startTime) > this->timeWindow) {

        int64_t timeSpan = static_cast<int64_t>(floor(span.count()));
        timeSpan = timeSpan - timeSpan % this->timeWindow.count();

        this->startTime += std::chrono::seconds(timeSpan);
        span = TimePoint::clock::now() - this->startTime;
        this->currentCount.store(0);

        resetLock.store(false);
    }

    if (++this->currentCount > this->rate && span < this->timeWindow) {
        sky_log("rate limiter hit: " + std::to_string(this->currentCount) + "/" + std::to_string(this->rate));
        return false;
    }

    return true;
}
