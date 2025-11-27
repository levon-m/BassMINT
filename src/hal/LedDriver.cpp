#include "hal/LedDriver.h"
#include "hal/BoardConfig.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

namespace BassMINT {

void LedDriver::init() {
    if (initialized_) {
        return;
    }

    // Initialize all LED pins as outputs
    for (uint8_t i = 0; i < NUM_STRINGS; ++i) {
        gpio_init(BoardConfig::LED_PINS[i]);
        gpio_set_dir(BoardConfig::LED_PINS[i], GPIO_OUT);
        gpio_put(BoardConfig::LED_PINS[i], 1); // Default to ON for testing
    }

    // TODO: Initialize PWM for brightness control
    // For now, simple digital on/off

    initialized_ = true;
}

void LedDriver::setLedOn(StringId string) {
    if (!initialized_) {
        return;
    }

    uint8_t index = static_cast<uint8_t>(string);
    if (index < NUM_STRINGS) {
        gpio_put(BoardConfig::LED_PINS[index], 1);
    }
}

void LedDriver::setLedOff(StringId string) {
    if (!initialized_) {
        return;
    }

    uint8_t index = static_cast<uint8_t>(string);
    if (index < NUM_STRINGS) {
        gpio_put(BoardConfig::LED_PINS[index], 0);
    }
}

void LedDriver::setLedBrightness(StringId string, uint8_t brightness) {
    if (!initialized_) {
        return;
    }

    uint8_t index = static_cast<uint8_t>(string);
    if (index >= NUM_STRINGS) {
        return;
    }

    brightness_[index] = brightness;

    // TODO: Implement PWM-based brightness control
    // For now, simple threshold
    if (brightness > 127) {
        gpio_put(BoardConfig::LED_PINS[index], 1);
    } else {
        gpio_put(BoardConfig::LED_PINS[index], 0);
    }
}

void LedDriver::allLedsOn() {
    for (uint8_t i = 0; i < NUM_STRINGS; ++i) {
        setLedOn(static_cast<StringId>(i));
    }
}

void LedDriver::allLedsOff() {
    for (uint8_t i = 0; i < NUM_STRINGS; ++i) {
        setLedOff(static_cast<StringId>(i));
    }
}

} // namespace BassMINT
