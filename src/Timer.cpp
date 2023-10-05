#include "Timer.h"

Timer::Timer() {
    start_time = std::chrono::high_resolution_clock::now();
}

long long Timer::elapsed() const {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - start_time).count();
    return duration; // return in microseconds
}
