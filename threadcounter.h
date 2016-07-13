#pragma once

#include <cstdint>
#include <atomic>

namespace imageresizer {

class ThreadManager {
public:
    static int32_t threadId() {
        return ++_threadCounter;
    }
private:
    static std::atomic_int _threadCounter;
};
} //namespace imageresizer
