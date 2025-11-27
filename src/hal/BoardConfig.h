#pragma once

#include <cstdint>

namespace BassMINT {
namespace BoardConfig {

/**
 * @brief Pin configuration for Seeed XIAO RP2040 + Adafruit MIDI FeatherWing
 *
 * XIAO RP2040 pinout reference:
 * - ADC0-3: GPIO26-29 (A0-A3 on XIAO silk)
 * - UART1 TX: GPIO4 (D6 on XIAO silk) - used for MIDI OUT
 * - PWM capable: Most GPIO pins
 *
 * Hardware connections:
 * - 4x OPT101P outputs → A0, A1, A2, A3 (GPIO26-29)
 * - 4x IR LED anodes → D0-D3 via 330Ω resistors
 * - MIDI FeatherWing TX connected to UART1 TX (GPIO4)
 */

// === ADC channels for OPT101P photodiode sensors ===
constexpr uint8_t ADC_PIN_STRING_E = 26;  // ADC0 / A0 - Low E string
constexpr uint8_t ADC_PIN_STRING_A = 27;  // ADC1 / A1 - A string
constexpr uint8_t ADC_PIN_STRING_D = 28;  // ADC2 / A2 - D string
constexpr uint8_t ADC_PIN_STRING_G = 29;  // ADC3 / A3 - G string

constexpr uint8_t ADC_PINS[4] = {
    ADC_PIN_STRING_E,
    ADC_PIN_STRING_A,
    ADC_PIN_STRING_D,
    ADC_PIN_STRING_G
};

// ADC channel numbers (GPIO26=ADC0, 27=ADC1, 28=ADC2, 29=ADC3)
constexpr uint8_t ADC_CHANNEL_STRING_E = 0;
constexpr uint8_t ADC_CHANNEL_STRING_A = 1;
constexpr uint8_t ADC_CHANNEL_STRING_D = 2;
constexpr uint8_t ADC_CHANNEL_STRING_G = 3;

// === IR LED control pins (PWM capable) ===
constexpr uint8_t LED_PIN_STRING_E = 0;   // D0 / GPIO0
constexpr uint8_t LED_PIN_STRING_A = 1;   // D1 / GPIO1
constexpr uint8_t LED_PIN_STRING_D = 2;   // D2 / GPIO2
constexpr uint8_t LED_PIN_STRING_G = 3;   // D3 / GPIO3

constexpr uint8_t LED_PINS[4] = {
    LED_PIN_STRING_E,
    LED_PIN_STRING_A,
    LED_PIN_STRING_D,
    LED_PIN_STRING_G
};

// === MIDI UART configuration ===
// Using UART1 for MIDI DIN output (31250 baud)
constexpr uint8_t UART_MIDI_TX_PIN = 4;   // D6 / GPIO4
constexpr uint8_t UART_MIDI_INSTANCE = 1; // UART1

// MIDI baud rate (standard DIN-5 MIDI)
constexpr uint32_t MIDI_BAUD_RATE = 31250;

// === Timing ===
// ADC sample timer interval in microseconds
// For 8kHz: 1000000 / 8000 = 125 µs
constexpr uint32_t ADC_TIMER_INTERVAL_US = 125;

} // namespace BoardConfig
} // namespace BassMINT
