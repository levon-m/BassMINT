/**
 * @file main.cpp
 * @brief BassMINT - Bass MIDI Controller with Intelligent Note Tracking
 *
 * Embedded C++ firmware for Seeed XIAO RP2040 based bass guitar MIDI controller.
 *
 * Hardware:
 * - Seeed XIAO RP2040 (RP2040 MCU)
 * - 4x OPT101P photodiode sensors (optical string detection)
 * - 4x TSAL6400 IR LEDs (940nm illumination)
 * - Adafruit MIDI FeatherWing (MIDI DIN output)
 *
 * Features:
 * - Real-time pitch detection using YIN algorithm
 * - 4-string bass support (E1, A1, D2, G2)
 * - Standard MIDI Note On/Off events
 * - Proprietary SysEx messages with string/fret information
 * - Low-latency (<20ms target)
 *
 * @author BassMINT Team
 * @date 2025
 */

#include "app/App.h"
#include "pico/stdlib.h"
#include <cstdio>

using namespace BassMINT;

int main() {
    // Initialize Pico SDK stdio (USB serial for debugging)
    stdio_init_all();

    // Small delay to allow USB serial to connect
    sleep_ms(500);

    printf("\n");
    printf("========================================\n");
    printf("  BassMINT - Bass MIDI Controller\n");
    printf("  Firmware v0.1.0\n");
    printf("========================================\n");
    printf("\n");

    // Create and initialize application
    App app;

    if (!app.init()) {
        printf("ERROR: Failed to initialize BassMINT!\n");
        printf("System halted.\n");
        while (true) {
            tight_loop_contents(); // Halt
        }
    }

    // Run main loop (never returns)
    app.run();

    // Should never reach here
    return 0;
}
