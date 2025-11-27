#pragma once

#include "core/Types.h"
#include <cstdint>

namespace BassMINT {

/**
 * @brief Driver for IR LED control (TSAL6400 940nm LEDs)
 *
 * Controls 4 IR LEDs (one per string) for optical sensing.
 * Supports simple on/off and future PWM for power/sensitivity tuning.
 */
class LedDriver {
public:
    /**
     * @brief Initialize all LED GPIO pins
     */
    void init();

    /**
     * @brief Turn on a specific LED at full brightness
     * @param string Which string's LED to control
     */
    void setLedOn(StringId string);

    /**
     * @brief Turn off a specific LED
     * @param string Which string's LED to control
     */
    void setLedOff(StringId string);

    /**
     * @brief Set LED brightness (0-255)
     * @param string Which string's LED to control
     * @param brightness 0=off, 255=full brightness
     *
     * @note PWM implementation is a TODO for future optimization
     */
    void setLedBrightness(StringId string, uint8_t brightness);

    /**
     * @brief Turn on all LEDs
     */
    void allLedsOn();

    /**
     * @brief Turn off all LEDs
     */
    void allLedsOff();

private:
    bool initialized_ = false;
    uint8_t brightness_[NUM_STRINGS] = {255, 255, 255, 255};
};

} // namespace BassMINT
