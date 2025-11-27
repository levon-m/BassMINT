#include "dsp/EnvelopeFollower.h"
#include <cmath>
#include <algorithm>

namespace BassMINT {

EnvelopeFollower::EnvelopeFollower(float sampleRate, float attackTimeMs, float releaseTimeMs)
    : sampleRate_(sampleRate)
    , envelope_(0.0f)
    , threshold_(0.1f)      // TODO: Tune for OPT101 output levels
    , hysteresisRatio_(0.7f) // Release at 70% of attack threshold
    , active_(false)
{
    attackCoeff_ = calcCoefficient(attackTimeMs);
    releaseCoeff_ = calcCoefficient(releaseTimeMs);
}

void EnvelopeFollower::update(float sample) {
    // Rectify signal (absolute value)
    float rectified = std::abs(sample);

    // Exponential smoothing (one-pole lowpass)
    // Use attack coefficient when signal rising, release when falling
    float coeff = (rectified > envelope_) ? attackCoeff_ : releaseCoeff_;
    envelope_ = coeff * rectified + (1.0f - coeff) * envelope_;

    // Hysteresis gate
    if (!active_) {
        // Activate if above attack threshold
        if (envelope_ > threshold_) {
            active_ = true;
        }
    } else {
        // Deactivate if below release threshold
        float releaseThreshold = threshold_ * hysteresisRatio_;
        if (envelope_ < releaseThreshold) {
            active_ = false;
        }
    }
}

void EnvelopeFollower::updateBlock(const float* samples, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        update(samples[i]);
    }
}

void EnvelopeFollower::reset() {
    envelope_ = 0.0f;
    active_ = false;
}

float EnvelopeFollower::calcCoefficient(float timeMs) const {
    // Calculate one-pole lowpass coefficient from time constant
    // coeff = 1 - exp(-1 / (timeConstant * sampleRate))
    // where timeConstant = timeMs / 1000
    float timeConstantSamples = (timeMs / 1000.0f) * sampleRate_;
    return 1.0f - std::exp(-1.0f / timeConstantSamples);
}

} // namespace BassMINT
