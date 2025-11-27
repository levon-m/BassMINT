#include "core/NoteMapping.h"
#include <cmath>
#include <algorithm>

namespace BassMINT {

float NoteMapping::getOpenStringFrequency(StringId string) {
    uint8_t index = static_cast<uint8_t>(string);
    if (index >= NUM_STRINGS) {
        return 0.0f;
    }
    return OPEN_STRING_FREQUENCIES[index];
}

int NoteMapping::frequencyToFret(StringId string, float frequencyHz) {
    if (frequencyHz <= 0.0f) {
        return -1;
    }

    float openStringFreq = getOpenStringFrequency(string);
    if (openStringFreq == 0.0f) {
        return -1;
    }

    // Calculate fret using equal temperament formula:
    // fret = 12 * log2(freq / f_open)
    float fretFloat = 12.0f * std::log2(frequencyHz / openStringFreq);

    // Round to nearest fret
    int fret = static_cast<int>(std::round(fretFloat));

    // Clamp to valid range
    if (fret < 0) {
        fret = 0;
    } else if (fret > MAX_FRET) {
        fret = MAX_FRET;
    }

    return fret;
}

uint8_t NoteMapping::fretToMidiNote(StringId string, int fret) {
    uint8_t index = static_cast<uint8_t>(string);
    if (index >= NUM_STRINGS) {
        return 0;
    }

    // Clamp fret to valid range
    int clampedFret = std::clamp(fret, 0, MAX_FRET);

    // MIDI note = open string note + fret
    int midiNote = OPEN_STRING_MIDI_NOTES[index] + clampedFret;

    // Clamp to valid MIDI range (0-127)
    return static_cast<uint8_t>(std::clamp(midiNote, 0, 127));
}

uint8_t NoteMapping::frequencyToMidiNote(float frequencyHz) {
    if (frequencyHz <= 0.0f) {
        return 0;
    }

    // Convert frequency to MIDI note using A4 (440 Hz = MIDI 69) as reference
    // MIDI note = 69 + 12 * log2(freq / 440)
    float noteFloat = static_cast<float>(A4_MIDI_NOTE) +
                     12.0f * std::log2(frequencyHz / A4_FREQUENCY);

    int note = static_cast<int>(std::round(noteFloat));

    // Clamp to valid MIDI range
    return static_cast<uint8_t>(std::clamp(note, 0, 127));
}

FretPosition NoteMapping::mapPitchToFret(StringId string, const PitchEstimate& pitch) {
    if (!pitch.isValid()) {
        return FretPosition(); // Invalid
    }

    int fret = frequencyToFret(string, pitch.frequencyHz);

    if (fret < 0) {
        return FretPosition(); // Invalid
    }

    return FretPosition(string, fret, pitch.frequencyHz, pitch.confidence);
}

uint8_t NoteMapping::getOpenStringMidiNote(StringId string) {
    uint8_t index = static_cast<uint8_t>(string);
    if (index >= NUM_STRINGS) {
        return 0;
    }
    return OPEN_STRING_MIDI_NOTES[index];
}

bool NoteMapping::isFrequencyPlausible(StringId string, float frequencyHz) {
    if (frequencyHz <= 0.0f) {
        return false;
    }

    float openStringFreq = getOpenStringFrequency(string);

    // Allow frequency from ~open string to ~24 frets up
    // 24 frets = 2 octaves = 4x frequency
    float minFreq = openStringFreq * 0.9f;  // 10% tolerance below open
    float maxFreq = openStringFreq * 4.5f;  // Slightly above 24th fret

    return (frequencyHz >= minFreq && frequencyHz <= maxFreq);
}

} // namespace BassMINT
