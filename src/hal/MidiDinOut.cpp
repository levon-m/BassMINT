#include "hal/MidiDinOut.h"
#include "hal/BoardConfig.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"

namespace BassMINT {

// MIDI status byte constants
static constexpr uint8_t MIDI_NOTE_OFF = 0x80;
static constexpr uint8_t MIDI_NOTE_ON = 0x90;
static constexpr uint8_t MIDI_CONTROL_CHANGE = 0xB0;

void MidiDinOut::init() {
    if (initialized_) {
        return;
    }

    // Initialize UART1 at MIDI baud rate (31250)
    uart_init(uart1, BoardConfig::MIDI_BAUD_RATE);

    // Set TX pin to UART mode
    gpio_set_function(BoardConfig::UART_MIDI_TX_PIN, GPIO_FUNC_UART);

    // MIDI is 8-N-1 (default for Pico SDK UART)
    // No need to configure data bits, stop bits, parity explicitly

    initialized_ = true;
}

void MidiDinOut::sendByte(uint8_t byte) {
    if (!initialized_) {
        return;
    }

    // Blocking write (MIDI is slow, this is fine)
    uart_putc_raw(uart1, byte);
}

void MidiDinOut::sendMessage(const uint8_t* data, size_t length) {
    if (!initialized_ || !data) {
        return;
    }

    for (size_t i = 0; i < length; ++i) {
        uart_putc_raw(uart1, data[i]);
    }
}

void MidiDinOut::sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    if (!initialized_) {
        return;
    }

    // Ensure valid ranges
    channel &= 0x0F;  // 0-15
    note &= 0x7F;     // 0-127
    velocity &= 0x7F; // 0-127

    uint8_t msg[3] = {
        static_cast<uint8_t>(MIDI_NOTE_ON | channel),
        note,
        velocity
    };
    sendMessage(msg, 3);
}

void MidiDinOut::sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
    if (!initialized_) {
        return;
    }

    channel &= 0x0F;
    note &= 0x7F;
    velocity &= 0x7F;

    uint8_t msg[3] = {
        static_cast<uint8_t>(MIDI_NOTE_OFF | channel),
        note,
        velocity
    };
    sendMessage(msg, 3);
}

void MidiDinOut::sendControlChange(uint8_t channel, uint8_t controller, uint8_t value) {
    if (!initialized_) {
        return;
    }

    channel &= 0x0F;
    controller &= 0x7F;
    value &= 0x7F;

    uint8_t msg[3] = {
        static_cast<uint8_t>(MIDI_CONTROL_CHANGE | channel),
        controller,
        value
    };
    sendMessage(msg, 3);
}

void MidiDinOut::sendSysEx(const uint8_t* data, size_t length) {
    if (!initialized_ || !data || length < 2) {
        return;
    }

    // SysEx should start with 0xF0 and end with 0xF7
    // Trust caller to format correctly
    sendMessage(data, length);
}

} // namespace BassMINT
