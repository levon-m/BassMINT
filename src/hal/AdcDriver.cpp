#include "hal/AdcDriver.h"
#include "hal/BoardConfig.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/time.h"

namespace BassMINT {

// ADC reference voltage (RP2040 ADC uses internal 3.3V reference)
static constexpr float ADC_VREF = 3.3f;
static constexpr float ADC_MAX_VALUE = 4095.0f; // 12-bit ADC

// Global instance pointer for ISR callback
static AdcDriver* g_adcDriverInstance = nullptr;

void AdcDriver::init() {
    if (initialized_) {
        return;
    }

    // Initialize ADC hardware
    adc_init();

    // Configure ADC GPIO pins (GPIO26-29 for ADC0-3)
    for (uint8_t i = 0; i < NUM_STRINGS; ++i) {
        adc_gpio_init(BoardConfig::ADC_PINS[i]);
    }

    // Set global instance for ISR
    g_adcDriverInstance = this;

    initialized_ = true;
}

void AdcDriver::setSampleCallback(SampleCallback callback) {
    sampleCallback_ = callback;
}

void AdcDriver::startSampling() {
    if (!initialized_ || sampling_) {
        return;
    }

    // Start repeating timer at configured interval
    // Negative interval means interval is in microseconds
    add_repeating_timer_us(
        -static_cast<int32_t>(BoardConfig::ADC_TIMER_INTERVAL_US),
        timerCallback,
        nullptr,
        &timer_
    );

    sampling_ = true;
}

void AdcDriver::stopSampling() {
    if (!sampling_) {
        return;
    }

    cancel_repeating_timer(&timer_);
    sampling_ = false;
}

uint16_t AdcDriver::readSingle(StringId string) {
    if (!initialized_) {
        return 0;
    }

    uint8_t channel = static_cast<uint8_t>(string);
    if (channel >= NUM_STRINGS) {
        return 0;
    }

    // Select ADC input
    adc_select_input(channel);

    // Read ADC (blocking)
    return adc_read();
}

float AdcDriver::rawToVoltage(uint16_t raw) {
    return (static_cast<float>(raw) / ADC_MAX_VALUE) * ADC_VREF;
}

bool AdcDriver::timerCallback(struct repeating_timer* t) {
    (void)t; // Unused
    if (g_adcDriverInstance) {
        return g_adcDriverInstance->onTimerFired();
    }
    return true; // Continue timer
}

bool AdcDriver::onTimerFired() {
    // ISR context - keep this FAST!

    // Select current channel
    adc_select_input(currentChannel_);

    // Read ADC (non-blocking, should be quick)
    uint16_t sample = adc_read();

    // Call user callback if registered
    if (sampleCallback_) {
        sampleCallback_(static_cast<StringId>(currentChannel_), sample);
    }

    // Round-robin to next channel
    currentChannel_ = (currentChannel_ + 1) % NUM_STRINGS;

    return true; // Continue timer
}

} // namespace BassMINT
