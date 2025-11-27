#pragma once

#include "core/Types.h"
#include <cstdint>

namespace BassMINT {

/**
 * @brief MIDI event structures and helpers
 *
 * Simple wrappers for MIDI events to make application code cleaner.
 */

/**
 * @brief MIDI Note On event
 */
struct MidiNoteOn {
    uint8_t channel;
    uint8_t note;
    uint8_t velocity;

    MidiNoteOn() : channel(0), note(0), velocity(0) {}

    MidiNoteOn(uint8_t ch, uint8_t n, uint8_t vel)
        : channel(ch), note(n), velocity(vel) {}
};

/**
 * @brief MIDI Note Off event
 */
struct MidiNoteOff {
    uint8_t channel;
    uint8_t note;
    uint8_t velocity;

    MidiNoteOff() : channel(0), note(0), velocity(0) {}

    MidiNoteOff(uint8_t ch, uint8_t n, uint8_t vel)
        : channel(ch), note(n), velocity(vel) {}
};

/**
 * @brief Helper to build MIDI events from fret positions
 */
class MidiEventBuilder {
public:
    /**
     * @brief Create Note On event from fret position
     * @param fretPos Fret position info
     * @param channel MIDI channel (0-15)
     * @param velocity MIDI velocity (1-127)
     * @return Note On event
     */
    static MidiNoteOn createNoteOn(const FretPosition& fretPos,
                                   uint8_t channel = MIDI_CHANNEL,
                                   uint8_t velocity = DEFAULT_VELOCITY);

    /**
     * @brief Create Note Off event from fret position
     * @param fretPos Fret position info
     * @param channel MIDI channel (0-15)
     * @param velocity Release velocity (0-127)
     * @return Note Off event
     */
    static MidiNoteOff createNoteOff(const FretPosition& fretPos,
                                     uint8_t channel = MIDI_CHANNEL,
                                     uint8_t velocity = 64);

    /**
     * @brief Create Note On from MIDI note number
     * @param midiNote MIDI note (0-127)
     * @param channel MIDI channel (0-15)
     * @param velocity MIDI velocity (1-127)
     * @return Note On event
     */
    static MidiNoteOn createNoteOn(uint8_t midiNote,
                                   uint8_t channel = MIDI_CHANNEL,
                                   uint8_t velocity = DEFAULT_VELOCITY);

    /**
     * @brief Create Note Off from MIDI note number
     * @param midiNote MIDI note (0-127)
     * @param channel MIDI channel (0-15)
     * @param velocity Release velocity (0-127)
     * @return Note Off event
     */
    static MidiNoteOff createNoteOff(uint8_t midiNote,
                                     uint8_t channel = MIDI_CHANNEL,
                                     uint8_t velocity = 64);
};

} // namespace BassMINT
