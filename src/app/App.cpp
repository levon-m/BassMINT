#include "app/App.h"
#include "hal/BoardConfig.h"
#include <cstdio>

namespace BassMINT {

App::App()
    : stringProcessors_{
        StringProcessor(StringId::E),
        StringProcessor(StringId::A),
        StringProcessor(StringId::D),
        StringProcessor(StringId::G)
    }
    , stringManagers_{
        StringManager(StringId::E, midiOut_),
        StringManager(StringId::A, midiOut_),
        StringManager(StringId::D, midiOut_),
        StringManager(StringId::G, midiOut_)
    }
    , loopCounter_(0)
    , lastStatsTime_(0)
{
}

bool App::init() {
    // Initialize timer (needed for stats)
    Timer::init();

    // Initialize LEDs
    ledDriver_.init();
    ledDriver_.allLedsOn(); // Turn on all IR LEDs

    // Initialize MIDI output
    midiOut_.init();

    // Initialize ADC
    adcDriver_.init();

    // Set ADC callback (captures 'this' for member function call)
    adcDriver_.setSampleCallback(
        [this](StringId stringId, uint16_t sample) {
            this->onAdcSample(stringId, sample);
        }
    );

    // Start ADC sampling
    adcDriver_.startSampling();

    lastStatsTime_ = Timer::getTimeMillis();

    printf("BassMINT initialized successfully!\n");
    printf("Sample rate: %lu Hz\n", SAMPLE_RATE_HZ);
    printf("Frame size: %lu samples\n", PITCH_FRAME_SIZE);
    printf("Ready to rock.\n");

    return true;
}

void App::run() {
    // Main loop - runs forever
    while (true) {
        tick();
    }
}

void App::tick() {
    // Process each string
    for (uint8_t i = 0; i < NUM_STRINGS; ++i) {
        // DSP processing (envelope, pitch detection)
        stringProcessors_[i].process();

        // MIDI event generation
        stringManagers_[i].update(stringProcessors_[i]);
    }

    // Increment loop counter
    loopCounter_++;

    // Print stats every second (optional, for debugging)
    uint32_t now = Timer::getTimeMillis();
    if (now - lastStatsTime_ >= 1000) {
        printStats();
        lastStatsTime_ = now;
        loopCounter_ = 0;
    }

    // Optional: yield to other tasks or sleep
    // For now, run as fast as possible to minimize latency
}

void App::shutdown() {
    // Stop ADC sampling
    adcDriver_.stopSampling();

    // Turn off all active notes
    for (uint8_t i = 0; i < NUM_STRINGS; ++i) {
        stringManagers_[i].forceNoteOff();
    }

    // Turn off LEDs
    ledDriver_.allLedsOff();

    printf("BassMINT shutdown complete.\n");
}

void App::onAdcSample(StringId stringId, uint16_t sample) {
    // ISR context - must be fast!
    uint8_t index = static_cast<uint8_t>(stringId);
    if (index < NUM_STRINGS) {
        stringProcessors_[index].pushSample(sample);
    }
}

void App::printStats() {
    // Print debug statistics (optional, disable for production)
    #ifdef BASSMINT_DEBUG_STATS
    printf("--- Stats (loops/sec: %u) ---\n", loopCounter_);

    for (uint8_t i = 0; i < NUM_STRINGS; ++i) {
        const char* stringNames[] = {"E", "A", "D", "G"};
        const auto& proc = stringProcessors_[i];
        const auto& mgr = stringManagers_[i];

        printf("String %s: buf=%zu, state=%d, note=%s (MIDI %u, fret %d)\n",
               stringNames[i],
               proc.getBufferLevel(),
               static_cast<int>(proc.getState()),
               mgr.isNoteOn() ? "ON " : "OFF",
               mgr.getCurrentMidiNote(),
               mgr.getCurrentFret());
    }
    #else
    // Minimal stats
    (void)loopCounter_; // Suppress unused warning
    #endif
}

} // namespace BassMINT
