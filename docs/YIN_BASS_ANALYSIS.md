# YIN Pitch Detection Analysis for Bass Guitar

## Critical Analysis: Can YIN Detect 40Hz Reliably?

### Current Configuration

```cpp
SAMPLE_RATE_HZ = 8000 Hz
PITCH_FRAME_SIZE = 512 samples
minFreq = 30 Hz
maxFreq = 400 Hz
```

### Mathematical Analysis for E1 (41.2 Hz)

#### Period Calculation
- **Frequency**: E1 = 41.203 Hz
- **Period**: T = 1/41.203 = **24.27 ms** = **194.2 samples** @ 8kHz

#### Window Coverage
- **Window size**: 512 samples = 64 ms
- **Cycles in window**: 512 / 194.2 = **2.64 cycles**

#### YIN Lag Range
```cpp
maxLag = sampleRate / minFreq = 8000 / 30 = 267 samples
minLag = sampleRate / maxFreq = 8000 / 400 = 20 samples

// Clamped to buffer constraints:
maxLag = min(267, bufferSize/2, MAX_LAG-1)
       = min(267, 256, 511)
       = 256 samples  // ⚠️ LIMITING FACTOR
```

### Problem Identification

#### Issue 1: Insufficient Cycles
**YIN Best Practice**: Requires 3-4 complete cycles for robust detection

- **Current**: 2.64 cycles for E1
- **Recommended**: ≥3.5 cycles
- **Status**: ⚠️ **BORDERLINE - May cause instability**

#### Issue 2: Lag Range Constraint
The `maxLag` is clamped to `bufferSize/2 = 256`:

```cpp
maxLag_ = std::min(maxLag_, bufferSize_ / 2);  // Line 21
```

**Why?** YIN difference function needs `bufferSize - tau` samples:
```cpp
for (size_t j = 0; j < bufferSize_ - tau; ++j) {
    float delta = samples[j] - samples[j + tau];
    sum += delta * delta;
}
```

For E1 detection (lag ~194 samples):
- Required samples: 194 + 194 = 388 samples minimum
- Available: 512 samples
- **Status**: ✅ **SUFFICIENT** (512 > 388)

However, for frequencies below ~41 Hz, we lose reliability:
- 35 Hz: lag = 229 samples, needs 458 samples (✅ OK)
- 31 Hz: lag = 258 samples, needs 516 samples (⚠️ **MARGINAL**)
- 30 Hz: lag = 267 samples, **CLAMPED TO 256** (❌ **FAILS**)

### Confidence Analysis

YIN confidence for bass frequencies depends on:

1. **Number of cycles**: More cycles → better CMNDF normalization
2. **Harmonic content**: Bass strings rich in harmonics help
3. **Signal stability**: Short windows → more variance

**Expected confidence for E1 with 512-sample window**: 0.6-0.8
- Below 0.7 threshold may cause intermittent detection
- Attack transients may take 2-3 frames to stabilize

---

## Recommendations

### Option 1: Increase Window Size (RECOMMENDED)

**Change in [src/core/Types.h](../src/core/Types.h:77):**

```cpp
// OLD:
constexpr uint32_t PITCH_FRAME_SIZE = 512;  // 64ms, 2.64 cycles @ E1

// NEW:
constexpr uint32_t PITCH_FRAME_SIZE = 1024; // 128ms, 5.3 cycles @ E1
```

**Impact:**
- ✅ E1 detection: 5.3 cycles (excellent)
- ✅ Confidence: 0.85-0.95 (very reliable)
- ✅ Can detect down to ~20 Hz if needed
- ⚠️ Latency: Increases from 64ms to 128ms
- ⚠️ Memory: +2KB per string (8KB total)

**Verdict**: **Best for reliability**, latency still acceptable for bass

---

### Option 2: Per-String Window Sizes

Use larger windows for low strings, smaller for high:

```cpp
// In StringProcessor constructor:
size_t windowSize = PITCH_FRAME_SIZE;
if (stringId == StringId::E) {
    windowSize = 1024;  // E string needs more cycles
} else if (stringId == StringId::A) {
    windowSize = 768;   // A string OK with 3.5 cycles
}
// D and G strings use default 512
```

**Impact:**
- ✅ Optimizes latency per string
- ✅ E string: reliable, G string: fast
- ⚠️ Complexity: Different processing paths
- ⚠️ Memory: Variable buffer sizes

**Verdict**: **Best for performance**, but more complex

---

### Option 3: Overlapping Windows (Advanced)

Process overlapping frames with 50% hop size:

```cpp
// 1024-sample window, 512-sample hop
// Gives 128ms window with 64ms update rate
```

**Impact:**
- ✅ Best of both worlds: reliability + low latency updates
- ✅ Smoother pitch tracking
- ⚠️ Complexity: Requires windowing (Hann/Hamming)
- ⚠️ CPU: 2x processing (but RP2040 can handle it)

**Verdict**: **Best overall**, implement later

---

### Option 4: Adaptive Thresholding

Keep 512 samples but lower confidence threshold for E string:

```cpp
// In StringProcessor:
if (stringId == StringId::E) {
    pitchDetector_.setConfidenceThreshold(0.12f); // More lenient
} else {
    pitchDetector_.setConfidenceThreshold(0.15f); // Default
}
```

**Impact:**
- ✅ No latency increase
- ✅ Simple change
- ⚠️ May accept false positives on E string
- ⚠️ Doesn't solve fundamental cycle count issue

**Verdict**: **Quick fix**, not ideal long-term

---

## Real-World Testing Data Needed

### Test Cases for E String (41.2 Hz)

| Fret | Frequency | Cycles @ 512 | Cycles @ 1024 | Expected Confidence |
|------|-----------|--------------|---------------|---------------------|
| 0    | 41.2 Hz   | 2.64         | 5.28          | 0.65 → 0.90        |
| 3    | 49.0 Hz   | 3.14         | 6.28          | 0.75 → 0.92        |
| 5    | 55.0 Hz   | 3.52         | 7.04          | 0.80 → 0.94        |
| 12   | 82.4 Hz   | 5.28         | 10.56         | 0.90 → 0.96        |

### Metrics to Capture

1. **Detection latency**: Time from pluck to first stable pitch
2. **Confidence distribution**: Histogram of confidence values
3. **False negatives**: Plucks that don't trigger within 200ms
4. **Pitch jitter**: Variance in detected frequency for sustained note

---

## Recommended Immediate Action

**Change `PITCH_FRAME_SIZE` to 1024 samples** before hardware testing.

Rationale:
- E1 is the **hardest case** (lowest frequency)
- 128ms latency is **acceptable for bass** (guitar amps have ~50-100ms anyway)
- Avoids tuning headaches during initial testing
- Can optimize later with per-string or overlapping windows

---

## Code Changes Required

### 1. Update [src/core/Types.h](../src/core/Types.h:77)

```cpp
/**
 * @brief Frame size for pitch detection
 *
 * 1024 samples @ 8kHz = 128ms window
 * - Sufficient for ~5 cycles of E1 (41Hz) → excellent YIN performance
 * - Tradeoff: Higher latency but much more reliable low-frequency detection
 * - Can reduce to 512 for D/G strings if needed (per-string optimization)
 */
constexpr uint32_t PITCH_FRAME_SIZE = 1024;
```

### 2. Update [src/dsp/PitchDetectorYin.h](../src/dsp/PitchDetectorYin.h:67)

```cpp
// Increase MAX_LAG to support 1024-sample windows
static constexpr size_t MAX_LAG = 1024; // Was 512
```

### 3. Update [ARCHITECTURE.md](../ARCHITECTURE.md) timing analysis

Document the new latency numbers:
- Sampling window: 128ms (was 64ms)
- Total worst-case latency: ~140ms (was ~80ms)
- Still well within acceptable range for bass

---

## References

- YIN paper: de Cheveigné & Kawahara (2002) - recommends 3+ cycles
- [Tartini](https://miracle.otago.ac.nz/tartini/papers.html) analysis tool uses 100-150ms windows for bass
- Commercial bass-to-MIDI converters (e.g., Fishman TriplePlay): 10-30ms but use DSP optimized for guitar

---

## Conclusion

**Current 512-sample window is BORDERLINE for E1 detection.**

✅ **Immediate fix**: Bump to 1024 samples
🔄 **Future optimization**: Per-string windows or overlapping frames
📊 **Validation**: Test with real bass and measure confidence distributions
