#include "core/SysExEncoder.h"
#include "core/NoteMapping.h"
#include <algorithm>

namespace BassMINT {

std::array<uint8_t, 10> SysExEncoder::encode(const FretSysExPayload& payload) {
    std::array<uint8_t, 10> message;

    // Ensure all data is 7-bit (MIDI SysEx requirement)
    uint8_t stringId = static_cast<uint8_t>(payload.string) & 0x7F;
    uint8_t fret = static_cast<uint8_t>(std::clamp(payload.fret, 0, 127)) & 0x7F;
    uint8_t midiNote = payload.midiNote & 0x7F;
    uint8_t velocity = payload.velocity & 0x7F;

    message[0] = SYSEX_START;
    message[1] = MANUFACTURER_ID;
    message[2] = BASSMINT_ID_1;
    message[3] = BASSMINT_ID_2;
    message[4] = PROTOCOL_VERSION;
    message[5] = stringId;
    message[6] = fret;
    message[7] = midiNote;
    message[8] = velocity;
    message[9] = SYSEX_END;

    return message;
}

SysExEncoder::FretSysExPayload
SysExEncoder::fromFretPosition(const FretPosition& fretPos, uint8_t velocity) {
    uint8_t midiNote = NoteMapping::fretToMidiNote(fretPos.string, fretPos.fret);
    return FretSysExPayload(fretPos.string, fretPos.fret, midiNote, velocity);
}

} // namespace BassMINT
