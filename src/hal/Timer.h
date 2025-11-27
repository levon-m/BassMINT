#pragma once

#include <cstdint>

namespace BassMINT {

/**
 * @brief Monotonic timer for timestamps and timing measurements
 *
 * Uses RP2040 hardware timer for microsecond-precision timestamps.
 * Thread-safe, can be called from ISR context.
 */
class Timer {
public:
    /**
     * @brief Initialize the timer subsystem
     */
    static void init();

    /**
     * @brief Get current time in microseconds since boot
     * @return Microsecond timestamp (wraps after ~71 minutes)
     */
    static uint32_t getTimeMicros();

    /**
     * @brief Get current time in milliseconds since boot
     * @return Millisecond timestamp
     */
    static uint32_t getTimeMillis();

    /**
     * @brief Calculate elapsed time in microseconds
     * @param startTime Previous timestamp from getTimeMicros()
     * @return Microseconds elapsed (handles wrap-around)
     */
    static uint32_t getElapsedMicros(uint32_t startTime);

    /**
     * @brief Busy-wait delay in microseconds
     * @param us Microseconds to delay
     */
    static void delayMicros(uint32_t us);

    /**
     * @brief Busy-wait delay in milliseconds
     * @param ms Milliseconds to delay
     */
    static void delayMillis(uint32_t ms);
};

} // namespace BassMINT
