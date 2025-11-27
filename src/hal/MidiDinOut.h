#pragma once

#include <cstdint>
#include <cstddef>

namespace BassMINT {

/**
 * @brief MIDI DIN output driver using RP2040 UART
 *
 * Implements standard MIDI 1.0 protocol over UART:
 * - 31250 baud, 8-N-1
 * - Sends raw MIDI bytes
 * - Convenience methods for Note On/Off
 *
 * Thread-safe: can be called from main loop (not ISR)
 */
class MidiDinOut {
public:
    /**
     * @brief Initialize MIDI UART
     * Sets up UART1 at 31250 baud on configured TX pin
     */
    void init();

    /**
     * @brief Send a single MIDI byte
     * @param byte Byte to transmit
     */
    void sendByte(uint8_t byte);

    /**
     * @brief Send a MIDI message (raw bytes)
     * @param data Pointer to message bytes
     * @param length Number of bytes to send
     */
    void sendMessage(const uint8_t* data, size_t length);

    /**
     * @brief Send MIDI Note On message
     * @param channel MIDI channel (0-15)
     * @param note MIDI note number (0-127)
     * @param velocity Velocity (1-127, 0 = note off)
     */
    void sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);

    /**
     * @brief Send MIDI Note Off message
     * @param channel MIDI channel (0-15)
     * @param note MIDI note number (0-127)
     * @param velocity Release velocity (0-127)
     */
    void sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);

    /**
     * @brief Send MIDI Control Change message
     * @param channel MIDI channel (0-15)
     * @param controller Controller number (0-127)
     * @param value Controller value (0-127)
     */
    void sendControlChange(uint8_t channel, uint8_t controller, uint8_t value);

    /**
     * @brief Send System Exclusive message
     * @param data SysEx payload (including F0 start and F7 end)
     * @param length Total message length
     */
    void sendSysEx(const uint8_t* data, size_t length);

private:
    bool initialized_ = false;
};

} // namespace BassMINT
