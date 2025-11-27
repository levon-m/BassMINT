#include "app/StringManager.h"
#include "core/NoteMapping.h"
#include "core/MidiEvents.h"

namespace BassMINT {

StringManager::StringManager(StringId stringId, MidiDinOut& midiOut)
    : stringId_(stringId)
    , midiOut_(midiOut)
    , noteOn_(false)
    , currentMidiNote_(0)
    , currentFret_(-1)
    , fretChangeCounter_(0)
    , pendingFret_(-1)
{
}

void StringManager::update(const StringProcessor& processor) {
    // Check if processor matches our string
    if (processor.getStringId() != stringId_) {
        return; // Wrong string
    }

    StringState state = processor.getState();
    const PitchEstimate& pitch = processor.getLatestPitch();

    // Map pitch to fret
    FretPosition currentFretPos;
    if (pitch.isValid() && processor.isActive()) {
        currentFretPos = NoteMapping::mapPitchToFret(stringId_, pitch);
    }

    // State machine
    switch (state) {
        case StringState::Attack:
            // String just became active
            if (currentFretPos.isValid()) {
                handleAttack(currentFretPos);
            }
            break;

        case StringState::Active:
            // String is actively vibrating
            if (currentFretPos.isValid()) {
                if (noteOn_) {
                    // Check for fret change
                    if (currentFretPos.fret != currentFret_) {
                        handleFretChange(currentFretPos);
                    } else {
                        // Reset fret change counter (stable)
                        fretChangeCounter_ = 0;
                        pendingFret_ = -1;
                    }
                } else {
                    // Note was off but should be on (late detection)
                    handleAttack(currentFretPos);
                }

                // Update last valid fret
                lastValidFret_ = currentFretPos;
            }
            break;

        case StringState::Release:
        case StringState::Idle:
            // String stopped vibrating
            if (noteOn_) {
                handleRelease();
            }
            break;
    }
}

void StringManager::forceNoteOff() {
    if (noteOn_) {
        sendNoteOff();
    }
}

void StringManager::handleAttack(const FretPosition& fretPos) {
    // String just became active with valid pitch
    sendNoteOn(fretPos);
}

void StringManager::handleRelease() {
    // String stopped vibrating
    sendNoteOff();
}

void StringManager::handleFretChange(const FretPosition& newFretPos) {
    // Debounce fret changes to avoid jitter
    if (newFretPos.fret == pendingFret_) {
        fretChangeCounter_++;
    } else {
        // New candidate fret
        pendingFret_ = newFretPos.fret;
        fretChangeCounter_ = 1;
    }

    // Accept fret change if stable for threshold frames
    if (fretChangeCounter_ >= FRET_CHANGE_THRESHOLD) {
        // Turn off old note
        sendNoteOff();

        // Turn on new note
        sendNoteOn(newFretPos);

        // Reset debounce
        fretChangeCounter_ = 0;
        pendingFret_ = -1;
    }
}

void StringManager::sendNoteOn(const FretPosition& fretPos) {
    uint8_t midiNote = NoteMapping::fretToMidiNote(fretPos.string, fretPos.fret);

    // Send MIDI Note On
    midiOut_.sendNoteOn(MIDI_CHANNEL, midiNote, DEFAULT_VELOCITY);

    // Send BassMINT SysEx
    auto sysexPayload = SysExEncoder::fromFretPosition(fretPos, DEFAULT_VELOCITY);
    auto sysexMsg = SysExEncoder::encode(sysexPayload);
    midiOut_.sendSysEx(sysexMsg.data(), sysexMsg.size());

    // Update state
    noteOn_ = true;
    currentMidiNote_ = midiNote;
    currentFret_ = fretPos.fret;
    lastValidFret_ = fretPos;
}

void StringManager::sendNoteOff() {
    if (!noteOn_) {
        return; // Already off
    }

    // Send MIDI Note Off
    midiOut_.sendNoteOff(MIDI_CHANNEL, currentMidiNote_, 64);

    // Update state
    noteOn_ = false;
    currentMidiNote_ = 0;
    currentFret_ = -1;
}

} // namespace BassMINT
