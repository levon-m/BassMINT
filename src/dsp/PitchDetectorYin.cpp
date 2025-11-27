#include "dsp/PitchDetectorYin.h"
#include <cmath>
#include <algorithm>
#include <cstring>

namespace BassMINT {

PitchDetectorYin::PitchDetectorYin(float sampleRate, size_t bufferSize,
                                   float minFreq, float maxFreq)
    : sampleRate_(sampleRate)
    , bufferSize_(bufferSize)
    , minFreq_(minFreq)
    , maxFreq_(maxFreq)
    , confidenceThreshold_(0.15f) // YIN default threshold
{
    // Calculate lag bounds from frequency range
    // lag = sampleRate / frequency
    maxLag_ = static_cast<size_t>(sampleRate_ / minFreq_);
    minLag_ = static_cast<size_t>(sampleRate_ / maxFreq_);

    // Clamp to buffer size and MAX_LAG
    maxLag_ = std::min(maxLag_, bufferSize_ / 2);
    maxLag_ = std::min(maxLag_, MAX_LAG - 1);
    minLag_ = std::max(minLag_, size_t(1));

    // Zero working buffers
    differenceFunction_.fill(0.0f);
    cmndf_.fill(0.0f);
}

PitchEstimate PitchDetectorYin::estimate(const float* samples, size_t count) {
    if (!samples || count != bufferSize_) {
        return PitchEstimate(); // Invalid input
    }

    // Step 1: Compute difference function
    computeDifference(samples);

    // Step 2: Compute cumulative mean normalized difference
    computeCMNDF();

    // Step 3: Absolute threshold to find period
    size_t tau = absoluteThreshold();

    if (tau == 0) {
        return PitchEstimate(); // No pitch detected
    }

    // Step 4: Parabolic interpolation for better accuracy
    float refinedTau = parabolicInterpolation(tau);

    // Convert lag to frequency
    float frequency = sampleRate_ / refinedTau;

    // Confidence = 1 - CMNDF at detected lag
    float confidence = 1.0f - cmndf_[tau];

    return PitchEstimate(frequency, confidence);
}

void PitchDetectorYin::computeDifference(const float* samples) {
    // YIN difference function: d(tau) = sum((samples[j] - samples[j+tau])^2)
    for (size_t tau = 0; tau < maxLag_; ++tau) {
        float sum = 0.0f;
        for (size_t j = 0; j < bufferSize_ - tau; ++j) {
            float delta = samples[j] - samples[j + tau];
            sum += delta * delta;
        }
        differenceFunction_[tau] = sum;
    }
}

void PitchDetectorYin::computeCMNDF() {
    // Cumulative mean normalized difference function
    // cmndf(0) = 1 by definition
    cmndf_[0] = 1.0f;

    float runningSum = 0.0f;

    for (size_t tau = 1; tau < maxLag_; ++tau) {
        runningSum += differenceFunction_[tau];

        if (runningSum == 0.0f) {
            cmndf_[tau] = 1.0f; // Avoid division by zero
        } else {
            cmndf_[tau] = differenceFunction_[tau] / (runningSum / static_cast<float>(tau));
        }
    }
}

size_t PitchDetectorYin::absoluteThreshold() {
    // Find first local minimum below threshold in valid lag range
    for (size_t tau = minLag_; tau < maxLag_ - 1; ++tau) {
        if (cmndf_[tau] < confidenceThreshold_) {
            // Check if local minimum (value less than neighbors)
            if (cmndf_[tau] < cmndf_[tau + 1]) {
                return tau;
            }
        }
    }

    // If no threshold crossing, find global minimum
    size_t minIdx = minLag_;
    float minVal = cmndf_[minLag_];

    for (size_t tau = minLag_ + 1; tau < maxLag_; ++tau) {
        if (cmndf_[tau] < minVal) {
            minVal = cmndf_[tau];
            minIdx = tau;
        }
    }

    // Only return if confidence is reasonable
    if (minVal < 0.5f) {
        return minIdx;
    }

    return 0; // No valid pitch
}

float PitchDetectorYin::parabolicInterpolation(size_t tau) {
    // Parabolic interpolation for sub-sample accuracy
    if (tau < 1 || tau >= maxLag_ - 1) {
        return static_cast<float>(tau); // Can't interpolate at boundaries
    }

    float s0 = cmndf_[tau - 1];
    float s1 = cmndf_[tau];
    float s2 = cmndf_[tau + 1];

    // Find vertex of parabola through three points
    float adjustment = (s2 - s0) / (2.0f * (2.0f * s1 - s2 - s0));

    return static_cast<float>(tau) + adjustment;
}

} // namespace BassMINT
