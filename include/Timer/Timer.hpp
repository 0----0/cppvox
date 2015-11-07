#include <chrono>
#include <thread>
#include <iostream>

using Clock = std::chrono::steady_clock;

class Timer {
private:
        const Clock::time_point start;
public:
        Timer(): start(Clock::now()) {}
        void roundTo(Clock::duration duration) {
                auto end = Clock::now();
                auto wait = start - end + duration;
                if(wait.count() > 0) {
                        std::this_thread::sleep_for(wait);
                }
        }
};
