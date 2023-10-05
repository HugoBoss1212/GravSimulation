#ifndef TIMER_H
#define TIMER_H

#include <chrono>

class Timer {
    std::chrono::high_resolution_clock::time_point start_time;
public:
    Timer();
    long long elapsed() const; // returns microseconds
};

#endif // TIMER_H