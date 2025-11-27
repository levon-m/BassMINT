#include "core/MidiEvents.h"
#include "core/NoteMapping.h"

namespace BassMINT {

MidiNoteOn MidiEventBuilder::createNoteOn(const FretPosition& fretPos,
                                          uint8_t channel, uint8_t velocity) {
    uint8_t midiNote = NoteMapping::fretToMidiNote(fretPos.string, fretPos.fret);
    return MidiNoteOn(channel, midiNote, velocity);
}

MidiNoteOff MidiEventBuilder::createNoteOff(const FretPosition& fretPos,
                                            uint8_t channel, uint8_t velocity) {
    uint8_t midiNote = NoteMapping::fretToMidiNote(fretPos.string, fretPos.fret);
    return MidiNoteOff(channel, midiNote, velocity);
}

MidiNoteOn MidiEventBuilder::createNoteOn(uint8_t midiNote,
                                          uint8_t channel, uint8_t velocity) {
    return MidiNoteOn(channel, midiNote, velocity);
}

MidiNoteOff MidiEventBuilder::createNoteOff(uint8_t midiNote,
                                            uint8_t channel, uint8_t velocity) {
    return MidiNoteOff(channel, midiNote, velocity);
}

} // namespace BassMINT
