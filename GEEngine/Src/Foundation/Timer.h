#pragma once
#include <chrono>

class Timer {
    std::chrono::high_resolution_clock::time_point last;

public:
    Timer() {
        last = std::chrono::high_resolution_clock::now();
    }

    float dt() {
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> delta = now - last;
        last = now;
        return delta.count();
    }
};
