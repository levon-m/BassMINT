#pragma once

#include "core/Types.h"
#include <cstdint>

namespace BassMINT {

/**
 * @brief Maps pitch frequencies to MIDI notes and fret positions
 *
 * Handles conversion between:
 * - Frequency (Hz) -> Fret number (for a given string)
 * - String + Fret -> MIDI note number
 * - Frequency -> Nearest MIDI note (generic, string-agnostic)
 *
 * Uses standard 4-string bass tuning (E1-A1-D2-G2) and equal temperament.
 */
class NoteMapping {
public:
    /**
     * @brief Get open string frequency
     * @param string Which string
     * @return Frequency in Hz
     */
    static float getOpenStringFrequency(StringId string);

    /**
     * @brief Convert frequency to fret number for a given string
     * @param string Which string
     * @param frequencyHz Detected frequency
     * @return Fret number (0 = open, 1-24 = fretted, -1 = invalid)
     */
    static int frequencyToFret(StringId string, float frequencyHz);

    /**
     * @brief Convert string + fret to MIDI note number
     * @param string Which string
     * @param fret Fret number (0-24)
     * @return MIDI note number (0-127)
     */
    static uint8_t fretToMidiNote(StringId string, int fret);

    /**
     * @brief Convert frequency to nearest MIDI note (string-agnostic)
     * @param frequencyHz Frequency in Hz
     * @return MIDI note number (0-127)
     */
    static uint8_t frequencyToMidiNote(float frequencyHz);

    /**
     * @brief Complete pitch-to-fret mapping
     * @param string Which string
     * @param pitch Pitch estimate from detector
     * @return Complete fret position info
     */
    static FretPosition mapPitchToFret(StringId string, const PitchEstimate& pitch);

    /**
     * @brief Get MIDI note number for open string
     * @param string Which string
     * @return MIDI note number
     */
    static uint8_t getOpenStringMidiNote(StringId string);

    /**
     * @brief Validate if frequency is plausible for a given string
     * @param string Which string
     * @param frequencyHz Detected frequency
     * @return true if frequency is within reasonable range for this string
     */
    static bool isFrequencyPlausible(StringId string, float frequencyHz);

private:
    // Open string frequencies (standard bass tuning)
    // E1 = 41.203 Hz (MIDI note 28)
    // A1 = 55.000 Hz (MIDI note 33)
    // D2 = 73.416 Hz (MIDI note 38)
    // G2 = 97.999 Hz (MIDI note 43)
    static constexpr float OPEN_STRING_FREQUENCIES[NUM_STRINGS] = {
        41.203f,  // E string
        55.000f,  // A string
        73.416f,  // D string
        97.999f   // G string
    };

    // MIDI note numbers for open strings
    static constexpr uint8_t OPEN_STRING_MIDI_NOTES[NUM_STRINGS] = {
        28,  // E1
        33,  // A1
        38,  // D2
        43   // G2
    };

    // Maximum fret number to consider
    static constexpr int MAX_FRET = 24;

    // A4 reference (440 Hz = MIDI note 69)
    static constexpr float A4_FREQUENCY = 440.0f;
    static constexpr uint8_t A4_MIDI_NOTE = 69;
};

} // namespace BassMINT
