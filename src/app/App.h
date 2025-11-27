#pragma once

#include "core/Types.h"
#include "app/StringManager.h"
#include "dsp/StringProcessor.h"
#include "hal/AdcDriver.h"
#include "hal/MidiDinOut.h"
#include "hal/LedDriver.h"
#include "hal/Timer.h"
#include <array>
#include <cstdint>

namespace BassMINT {

/**
 * @brief Top-level application orchestrator for BassMINT
 *
 * Responsibilities:
 * - Initialize all hardware (ADC, MIDI, LEDs, timers)
 * - Manage 4 StringProcessor instances (one per string)
 * - Manage 4 StringManager instances (MIDI event generation)
 * - Coordinate ADC sampling -> DSP processing -> MIDI output pipeline
 * - Main loop execution
 *
 * This is the "god object" that ties everything together.
 */
class App {
public:
    /**
     * @brief Constructor
     */
    App();

    /**
     * @brief Initialize all hardware and subsystems
     * @return true if initialization succeeded
     */
    bool init();

    /**
     * @brief Main application loop (blocking, runs forever)
     */
    void run();

    /**
     * @brief Single iteration of main loop (for testing)
     */
    void tick();

    /**
     * @brief Stop all processing and turn off notes
     */
    void shutdown();

private:
    // HAL drivers
    AdcDriver adcDriver_;
    MidiDinOut midiOut_;
    LedDriver ledDriver_;

    // Per-string DSP processors
    std::array<StringProcessor, NUM_STRINGS> stringProcessors_;

    // Per-string MIDI managers
    std::array<StringManager, NUM_STRINGS> stringManagers_;

    // Statistics / monitoring
    uint32_t loopCounter_;
    uint32_t lastStatsTime_;

    /**
     * @brief ADC sample callback (called from ISR)
     * Pushes samples into appropriate string processor
     */
    void onAdcSample(StringId stringId, uint16_t sample);

    /**
     * @brief Print debug statistics (if USB serial enabled)
     */
    void printStats();
};

} // namespace BassMINT
