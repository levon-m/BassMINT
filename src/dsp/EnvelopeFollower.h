#pragma once

#include <cstdint>
#include <cmath>

namespace BassMINT {

/**
 * @brief Envelope follower for string activity detection
 *
 * Tracks amplitude envelope of incoming audio signal using:
 * - Rectification (absolute value or square)
 * - Exponential smoothing (simple one-pole lowpass)
 * - Hysteresis-based gate (attack/release thresholds)
 *
 * Used to determine when a bass string is vibrating vs. idle.
 */
class EnvelopeFollower {
public:
    /**
     * @brief Constructor
     * @param sampleRate Sample rate in Hz
     * @param attackTimeMs Attack time constant in milliseconds
     * @param releaseTimeMs Release time constant in milliseconds
     */
    EnvelopeFollower(float sampleRate = 8000.0f,
                     float attackTimeMs = 10.0f,
                     float releaseTimeMs = 100.0f);

    /**
     * @brief Update envelope with new sample
     * @param sample Input sample (raw ADC or normalized)
     */
    void update(float sample);

    /**
     * @brief Update envelope with block of samples
     * @param samples Input samples
     * @param count Number of samples
     */
    void updateBlock(const float* samples, size_t count);

    /**
     * @brief Get current envelope value
     * @return Current envelope amplitude (smoothed)
     */
    float getEnvelope() const { return envelope_; }

    /**
     * @brief Check if signal is active (above threshold)
     * @return true if string is vibrating
     */
    bool isActive() const { return active_; }

    /**
     * @brief Set activation threshold
     * @param threshold Amplitude threshold for "active" state
     */
    void setThreshold(float threshold) { threshold_ = threshold; }

    /**
     * @brief Set hysteresis ratio
     * @param ratio Release threshold = attack threshold * ratio (0.0-1.0)
     */
    void setHysteresis(float ratio) { hysteresisRatio_ = ratio; }

    /**
     * @brief Reset envelope state
     */
    void reset();

private:
    float sampleRate_;
    float envelope_;        // Current envelope value
    float attackCoeff_;     // Attack smoothing coefficient
    float releaseCoeff_;    // Release smoothing coefficient
    float threshold_;       // Activation threshold
    float hysteresisRatio_; // Release threshold = threshold * hysteresisRatio
    bool active_;           // Current gate state

    /**
     * @brief Calculate smoothing coefficient from time constant
     * @param timeMs Time constant in milliseconds
     * @return Smoothing coefficient (0.0-1.0)
     */
    float calcCoefficient(float timeMs) const;
};

} // namespace BassMINT
