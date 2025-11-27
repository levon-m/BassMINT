#include "hal/Timer.h"
#include "pico/time.h"

namespace BassMINT {

void Timer::init() {
    // Pico SDK initializes the timer automatically
    // Nothing needed here, but keep for API consistency
}

uint32_t Timer::getTimeMicros() {
    return to_us_since_boot(get_absolute_time());
}

uint32_t Timer::getTimeMillis() {
    return to_ms_since_boot(get_absolute_time());
}

uint32_t Timer::getElapsedMicros(uint32_t startTime) {
    uint32_t now = getTimeMicros();
    // Handle 32-bit wrap-around
    return now - startTime;
}

void Timer::delayMicros(uint32_t us) {
    sleep_us(us);
}

void Timer::delayMillis(uint32_t ms) {
    sleep_ms(ms);
}

} // namespace BassMINT
