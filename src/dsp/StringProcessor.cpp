#include "dsp/StringProcessor.h"
#include <algorithm>

namespace BassMINT {

// ADC midpoint for DC removal (12-bit ADC, centered at 2048)
static constexpr float ADC_MIDPOINT = 2048.0f;
static constexpr float ADC_SCALE = 1.0f / 2048.0f;

StringProcessor::StringProcessor(StringId stringId, float sampleRate)
    : stringId_(stringId)
    , sampleRate_(sampleRate)
    , state_(StringState::Idle)
    , envelopeFollower_(sampleRate)
    , pitchDetector_(sampleRate, PITCH_FRAME_SIZE)
    , wasActive_(false)
{
    floatBuffer_.fill(0.0f);
    rawBuffer_.fill(0);

    // TODO: Tune envelope follower parameters per-string if needed
    // Different strings may have different optical characteristics
    envelopeFollower_.setThreshold(0.15f);  // Adjust based on testing
    envelopeFollower_.setHysteresis(0.6f);
}

bool StringProcessor::pushSample(uint16_t rawSample) {
    // ISR context - must be fast!
    return sampleBuffer_.push(rawSample);
}

void StringProcessor::process() {
    // Main loop context

    // Read available samples from ring buffer
    size_t available = sampleBuffer_.getAvailable();

    if (available == 0) {
        return; // Nothing to process
    }

    // Process samples in chunks up to frame size
    size_t toProcess = std::min(available, PITCH_FRAME_SIZE);

    // Read from ring buffer
    size_t read = sampleBuffer_.read(rawBuffer_.data(), toProcess);

    if (read == 0) {
        return;
    }

    // Convert to float and update envelope
    for (size_t i = 0; i < read; ++i) {
        floatBuffer_[i] = normalizeAdcSample(rawBuffer_[i]);
        envelopeFollower_.update(floatBuffer_[i]);
    }

    // Update state machine
    updateState();

    // Run pitch detection if:
    // - String is active
    // - We have a full frame
    if (isActive() && read >= PITCH_FRAME_SIZE) {
        latestPitch_ = pitchDetector_.estimate(floatBuffer_.data(), PITCH_FRAME_SIZE);

        // Optionally reject low-confidence estimates
        if (latestPitch_.confidence < MIN_PITCH_CONFIDENCE) {
            latestPitch_ = PitchEstimate(); // Invalidate
        }
    }
}

void StringProcessor::reset() {
    sampleBuffer_.clear();
    envelopeFollower_.reset();
    state_ = StringState::Idle;
    wasActive_ = false;
    latestPitch_ = PitchEstimate();
}

float StringProcessor::normalizeAdcSample(uint16_t raw) const {
    // Remove DC bias and normalize to [-1.0, 1.0]
    // OPT101 output is biased around Vcc/2, so ADC reads ~2048 at rest
    float centered = static_cast<float>(raw) - ADC_MIDPOINT;
    return centered * ADC_SCALE;
}

void StringProcessor::updateState() {
    bool currentlyActive = envelopeFollower_.isActive();

    // State transitions
    if (!wasActive_ && currentlyActive) {
        // Idle -> Attack
        state_ = StringState::Attack;
    } else if (wasActive_ && !currentlyActive) {
        // Active -> Release
        state_ = StringState::Release;
    } else if (currentlyActive) {
        // Attack -> Active (after first frame)
        if (state_ == StringState::Attack) {
            state_ = StringState::Active;
        }
    } else {
        // Release -> Idle
        if (state_ == StringState::Release) {
            state_ = StringState::Idle;
            latestPitch_ = PitchEstimate(); // Clear pitch on idle
        }
    }

    wasActive_ = currentlyActive;
}

} // namespace BassMINT
