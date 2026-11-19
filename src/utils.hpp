#ifndef UTILS_H
#define UTILS_H

#include <chrono>
#include <string>
#include <iostream>

// #define DEBUG_TIME

class Profiler {
public:
    using Clock = std::chrono::steady_clock;
    Clock::time_point last = Clock::now();

    void mark(const std::string &name) {
        #ifndef DEBUG_TIME
            return;
        #endif
        auto now = Clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(now - last).count();
        std::cout << "[PROFILE] " << name << ": " << elapsed << " ms" << std::endl;

        last = now;
    }
};

#endif