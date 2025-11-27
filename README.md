# **Bass** **M**ount for **I**nfared **N**ote **T**ranscription

An embedded C++ device for transcribing 4-string bass guitar tablature using optical string sensing, real-time pitch detection, and MIDI.

## Overview

BassMINT provides:

- **Real-time pitch detection** using the YIN algorithm
- **Per-string sensing** via OPT101P photodiodes and IR LEDs
- **Standard MIDI output** (Note On/Off events)
- **Proprietary SysEx messages** encoding string index + fret number
- **Sub-20ms latency** target for responsive playing

### Hardware

- **MCU**: Seeed XIAO RP2040 (RP2040 dual-core ARM Cortex-M0+)
- **Sensors**: 4× OPT101P integrated photodiode + TIA
- **LEDs**: 4× TSAL6400 940nm infrared LEDs
- **MIDI**: Adafruit MIDI FeatherWing (MIDI DIN output)
- **Tuning**: Standard 4-string bass (E1, A1, D2, G2)

### Architecture

```
Hardware Layer (HAL)
├── AdcDriver       - 4-channel ADC sampling @ 8kHz
├── MidiDinOut      - UART @ 31250 baud
├── LedDriver       - IR LED control
└── Timer           - Microsecond timestamps

DSP Layer
├── RingBuffer      - Lock-free sample buffering (ISR → main)
├── EnvelopeFollower - String activity detection
├── PitchDetectorYin - YIN pitch estimation (30-400 Hz)
└── StringProcessor  - Per-string processing chain

Core Logic
├── NoteMapping     - Frequency → fret/MIDI note conversion
├── MidiEvents      - MIDI event builders
└── SysExEncoder    - BassMINT SysEx protocol

Application Layer
├── StringManager   - Per-string MIDI event generation
└── App             - Main orchestrator
```

## Building

### Prerequisites

1. **Pico SDK** (v1.5.0+)
   ```bash
   git clone https://github.com/raspberrypi/pico-sdk.git
   cd pico-sdk
   git submodule update --init
   export PICO_SDK_PATH=$(pwd)
   ```

2. **Toolchain**
   - `arm-none-eabi-gcc` (10.3.1+)
   - CMake 3.13+
   - Make or Ninja

### Build Steps

```bash
# Configure
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc)

# Output: bassmint.uf2
```

### Flashing

1. Hold BOOT button on XIAO RP2040 while plugging in USB
2. Copy `build/bassmint.uf2` to the mounted drive
3. Board will reboot automatically

## Pin Configuration

See [src/hal/BoardConfig.h](src/hal/BoardConfig.h) for complete mapping.

### ADC Inputs (OPT101P)

| String | ADC Pin | GPIO |
|--------|---------|------|
| E      | A0      | 26   |
| A      | A1      | 27   |
| D      | A2      | 28   |
| G      | A3      | 29   |

### LED Outputs (TSAL6400)

| String | Digital Pin | GPIO |
|--------|-------------|------|
| E      | D0          | 0    |
| A      | D1          | 1    |
| D      | D2          | 2    |
| G      | D3          | 3    |

### MIDI

- **TX**: D6 (GPIO4) → UART1 @ 31250 baud

## MIDI Protocol

### Standard MIDI

BassMINT sends standard MIDI 1.0 Note On/Off events on channel 1:

- **Note On**: `0x90 <note> <velocity>`
- **Note Off**: `0x80 <note> <velocity>`

### BassMINT SysEx

Custom SysEx messages provide string + fret information:

```
F0 7D 'B' 'M' 01 <string> <fret> <note> <velocity> F7
```

| Byte | Description |
|------|-------------|
| `F0` | SysEx start |
| `7D` | Non-commercial manufacturer ID |
| `'B'` `'M'` | "BassMINT" identifier |
| `01` | Protocol version |
| `<string>` | String index (0=E, 1=A, 2=D, 3=G) |
| `<fret>` | Fret number (0-24) |
| `<note>` | MIDI note number (redundant) |
| `<velocity>` | Velocity (0-127) |
| `F7` | SysEx end |

## Configuration

### Sample Rate

Defined in [src/core/Types.h](src/core/Types.h):

```cpp
constexpr uint32_t SAMPLE_RATE_HZ = 8000;  // 8 kHz per channel
constexpr uint32_t PITCH_FRAME_SIZE = 512; // 64ms window
```

**Tradeoff**: 8kHz chosen for:
- Nyquist = 4kHz (well above 400Hz bass range)
- 512-sample window = 64ms latency
- Low CPU/memory usage

### Pitch Detection

YIN algorithm parameters in [src/dsp/PitchDetectorYin.cpp](src/dsp/PitchDetectorYin.cpp):

```cpp
float minFreq = 30.0f;   // Below E1 for headroom
float maxFreq = 400.0f;  // Above typical bass range
float confidenceThreshold = 0.15f; // YIN default
```

### Envelope Follower

Attack/release times in [src/dsp/StringProcessor.cpp](src/dsp/StringProcessor.cpp):

```cpp
envelopeFollower_.setThreshold(0.15f);  // TODO: Tune for OPT101 levels
envelopeFollower_.setHysteresis(0.6f);  // Release at 60% of attack
```

## Known Limitations / TODOs

### Current Limitations

- **Monophonic per string**: No polyphonic detection (intentional for v1)
- **No velocity sensing**: Constant velocity (TODO: add envelope-based velocity)
- **Fixed LED brightness**: No adaptive IR power (TODO: PWM implementation)
- **No calibration**: Envelope thresholds hardcoded (TODO: auto-calibration routine)

## References

- **YIN Algorithm**: A. de Cheveigné and H. Kawahara, "YIN, a fundamental frequency estimator for speech and music," JASA, 2002.
- **OPT101 Datasheet**: Texas Instruments
- **RP2040 Datasheet**: Raspberry Pi Foundation
- **MIDI 1.0 Specification**: MIDI Manufacturers Association
