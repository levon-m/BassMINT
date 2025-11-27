#pragma once

#include "core/Types.h"
#include <cstdint>
#include <array>

namespace BassMINT {

/**
 * @brief BassMINT proprietary SysEx message encoder
 *
 * Format:
 *   F0                   - SysEx start
 *   7D                   - Non-commercial manufacturer ID
 *   'B' 'M'              - "BassMINT" identifier
 *   0x01                 - Protocol version
 *   stringId             - String index (0=E, 1=A, 2=D, 3=G)
 *   fret                 - Fret number (0-24, 7-bit)
 *   midiNote             - MIDI note number (0-127)
 *   velocity             - Velocity (0-127)
 *   F7                   - SysEx end
 *
 * Total: 10 bytes
 *
 * This allows DAW plugins to know exactly which string and fret was played,
 * enabling realistic bass guitar emulation beyond just MIDI notes.
 */
class SysExEncoder {
public:
    /**
     * @brief Payload for BassMINT fret SysEx message
     */
    struct FretSysExPayload {
        StringId string;
        int fret;           // 0-24
        uint8_t midiNote;   // 0-127 (redundant but convenient)
        uint8_t velocity;   // 0-127

        FretSysExPayload()
            : string(StringId::E), fret(0), midiNote(0), velocity(0) {}

        FretSysExPayload(StringId s, int f, uint8_t note, uint8_t vel)
            : string(s), fret(f), midiNote(note), velocity(vel) {}
    };

    /**
     * @brief Encode fret position into SysEx message
     * @param payload Fret position data
     * @return SysEx message bytes (10 bytes total)
     */
    static std::array<uint8_t, 10> encode(const FretSysExPayload& payload);

    /**
     * @brief Create payload from fret position
     * @param fretPos Fret position from pitch detection
     * @param velocity MIDI velocity
     * @return SysEx payload ready for encoding
     */
    static FretSysExPayload fromFretPosition(const FretPosition& fretPos,
                                             uint8_t velocity = DEFAULT_VELOCITY);

private:
    // SysEx constants
    static constexpr uint8_t SYSEX_START = 0xF0;
    static constexpr uint8_t SYSEX_END = 0xF7;
    static constexpr uint8_t MANUFACTURER_ID = 0x7D; // Non-commercial
    static constexpr uint8_t BASSMINT_ID_1 = 'B';
    static constexpr uint8_t BASSMINT_ID_2 = 'M';
    static constexpr uint8_t PROTOCOL_VERSION = 0x01;
};

} // namespace BassMINT
