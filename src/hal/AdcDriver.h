#pragma once

#include "core/Types.h"
#include <cstdint>
#include <functional>
#include "pico/time.h"

namespace BassMINT {

/**
 * @brief ADC driver for 4-channel string sensing
 *
 * Manages timer-driven ADC sampling of 4 OPT101 photodiode outputs.
 * Uses hardware timer + ADC round-robin to sample all 4 channels at SAMPLE_RATE_HZ.
 *
 * Architecture:
 * - Timer IRQ triggers at SAMPLE_RATE_HZ
 * - Each IRQ reads one ADC channel in round-robin fashion
 * - Samples are pushed to per-string callbacks (typically ring buffers)
 * - ISR is kept minimal for low latency
 */
class AdcDriver {
public:
    /**
     * @brief Callback for new ADC sample
     * @param stringId Which string this sample is from
     * @param sample Raw ADC value (12-bit, 0-4095)
     */
    using SampleCallback = std::function<void(StringId stringId, uint16_t sample)>;

    /**
     * @brief Initialize ADC hardware and GPIO pins
     */
    void init();

    /**
     * @brief Set callback for ADC samples
     * @param callback Function to call when new sample arrives (from ISR context!)
     */
    void setSampleCallback(SampleCallback callback);

    /**
     * @brief Start timer-driven ADC sampling
     * Begins calling the sample callback at SAMPLE_RATE_HZ per channel
     */
    void startSampling();

    /**
     * @brief Stop ADC sampling
     */
    void stopSampling();

    /**
     * @brief Check if sampling is active
     */
    bool isSampling() const { return sampling_; }

    /**
     * @brief Read a single ADC sample (blocking, for testing)
     * @param string Which string to sample
     * @return 12-bit ADC value (0-4095)
     */
    uint16_t readSingle(StringId string);

    /**
     * @brief Convert raw ADC value to voltage
     * @param raw 12-bit ADC value (0-4095)
     * @return Voltage (0.0 - 3.3V)
     */
    static float rawToVoltage(uint16_t raw);

private:
    /**
     * @brief Timer ISR callback (static for C linkage)
     */
    static bool timerCallback(struct repeating_timer* t);

    /**
     * @brief Instance method called from timer ISR
     */
    bool onTimerFired();

    SampleCallback sampleCallback_;
    bool initialized_ = false;
    bool sampling_ = false;

    // Round-robin state
    uint8_t currentChannel_ = 0;

    // Timer handle
    struct repeating_timer timer_;
};

} // namespace BassMINT
