#pragma once

#include "core/Types.h"
#include "core/MidiEvents.h"
#include "core/SysExEncoder.h"
#include "dsp/StringProcessor.h"
#include "hal/MidiDinOut.h"
#include <cstdint>

namespace BassMINT {

/**
 * @brief Manages MIDI event generation for a single string
 *
 * Responsibilities:
 * - Track current note state (on/off, which fret)
 * - Detect note changes (fret changes, string attack/release)
 * - Generate MIDI Note On/Off events
 * - Generate BassMINT SysEx messages
 * - Handle note hysteresis/debouncing
 *
 * One instance per string, coordinated by App.
 */
class StringManager {
public:
    /**
     * @brief Constructor
     * @param stringId Which string this manager handles
     * @param midiOut Reference to MIDI output driver
     */
    StringManager(StringId stringId, MidiDinOut& midiOut);

    /**
     * @brief Update state and generate MIDI events
     * @param processor String processor with latest pitch/state
     *
     * Call this every main loop iteration with updated processor state
     */
    void update(const StringProcessor& processor);

    /**
     * @brief Force note off (emergency stop)
     */
    void forceNoteOff();

    /**
     * @brief Get current MIDI note (or 0 if none)
     */
    uint8_t getCurrentMidiNote() const { return currentMidiNote_; }

    /**
     * @brief Check if note is currently on
     */
    bool isNoteOn() const { return noteOn_; }

    /**
     * @brief Get current fret (or -1 if none)
     */
    int getCurrentFret() const { return currentFret_; }

private:
    StringId stringId_;
    MidiDinOut& midiOut_;

    // Current state
    bool noteOn_;
    uint8_t currentMidiNote_;
    int currentFret_;
    FretPosition lastValidFret_;

    // Hysteresis for fret changes
    int fretChangeCounter_;
    int pendingFret_;
    static constexpr int FRET_CHANGE_THRESHOLD = 3; // Frames before accepting fret change

    /**
     * @brief Handle string attack (idle -> active)
     */
    void handleAttack(const FretPosition& fretPos);

    /**
     * @brief Handle string release (active -> idle)
     */
    void handleRelease();

    /**
     * @brief Handle fret change while note is on
     */
    void handleFretChange(const FretPosition& newFretPos);

    /**
     * @brief Send MIDI Note On + SysEx
     */
    void sendNoteOn(const FretPosition& fretPos);

    /**
     * @brief Send MIDI Note Off
     */
    void sendNoteOff();
};

} // namespace BassMINT
