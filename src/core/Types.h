#pragma once

#include <cstdint>

namespace BassMINT {

/**
 * @brief Identifies each of the 4 bass strings
 * Standard bass tuning: E1-A1-D2-G2
 */
enum class StringId : uint8_t {
    E = 0,  // Low E string  (~41.2 Hz)
    A = 1,  // A string      (~55.0 Hz)
    D = 2,  // D string      (~73.4 Hz)
    G = 3   // High G string (~98.0 Hz)
};

constexpr uint8_t NUM_STRINGS = 4;

/**
 * @brief Result of pitch detection algorithm
 */
struct PitchEstimate {
    float frequencyHz;  // Detected fundamental frequency
    float confidence;   // 0.0 to 1.0, higher = more confident

    PitchEstimate() : frequencyHz(0.0f), confidence(0.0f) {}
    PitchEstimate(float freq, float conf) : frequencyHz(freq), confidence(conf) {}

    bool isValid() const { return confidence > 0.0f; }
};

/**
 * @brief Complete fret position information
 */
struct FretPosition {
    StringId string;
    int fret;           // 0 = open string, 1-24 = fretted
    float frequency;    // Actual detected frequency
    float confidence;   // From pitch detector

    FretPosition()
        : string(StringId::E), fret(0), frequency(0.0f), confidence(0.0f) {}

    FretPosition(StringId s, int f, float freq, float conf)
        : string(s), fret(f), frequency(freq), confidence(conf) {}

    bool isValid() const { return confidence > 0.0f && fret >= 0; }
};

/**
 * @brief State of a single string
 */
enum class StringState : uint8_t {
    Idle,       // Not vibrating
    Active,     // Vibrating, pitch being tracked
    Attack,     // Transition from idle to active
    Release     // Transition from active to idle
};

/**
 * @brief Sample rate configuration
 *
 * 8kHz chosen as reasonable tradeoff:
 * - Nyquist = 4kHz, well above highest bass fundamental (~400Hz for G string high frets)
 * - Frame size ~256 samples = 32ms latency (acceptable for bass)
 * - Lower CPU/memory than 16kHz or 44.1kHz
 * - RP2040 has plenty of headroom for 4 channels @ 8kHz
 */
constexpr uint32_t SAMPLE_RATE_HZ = 8000;

/**
 * @brief Frame size for pitch detection
 *
 * 1024 samples @ 8kHz = 128ms window
 * - Provides ~5.3 cycles of E1 (41Hz) for robust YIN detection
 * - YIN requires 3-4 cycles minimum; this ensures excellent confidence
 * - Tradeoff: Higher latency but much more reliable for low E string
 * - See docs/YIN_BASS_ANALYSIS.md for detailed analysis
 */
constexpr uint32_t PITCH_FRAME_SIZE = 1024;

/**
 * @brief Ring buffer size per string (must be power of 2)
 *
 * 1024 samples @ 8kHz = 128ms buffering
 * Allows for ~2 full pitch frames in buffer
 */
constexpr uint32_t RING_BUFFER_SIZE = 1024;

// Confidence threshold for accepting pitch detection
constexpr float MIN_PITCH_CONFIDENCE = 0.7f;

// MIDI configuration
constexpr uint8_t MIDI_CHANNEL = 0;  // MIDI channel 1 (0-indexed)
constexpr uint8_t DEFAULT_VELOCITY = 100;

} // namespace BassMINT
