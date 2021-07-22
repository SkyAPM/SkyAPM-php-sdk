#ifndef SKYWALKING_SKY_RATE_LIMIT_H
#define SKYWALKING_SKY_RATE_LIMIT_H

#include <cstdint>
#include <chrono>
#include <atomic>

class FixedWindowRateLimitor {
public:
    using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

    FixedWindowRateLimitor(int64_t rate, int seconds = 1);
    bool validate();

private:
    std::atomic_int64_t currentCount;
    int64_t rate;
    std::chrono::duration<int> timeWindow; // second
    TimePoint startTime;

    std::atomic_bool resetLock;
};

#endif // SKYWALKING_SKY_RATE_LIMIT_H
