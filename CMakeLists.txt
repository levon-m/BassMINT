cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
include(cmake/pico_sdk_import.cmake)

project(bassmint C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# Create the executable
add_executable(bassmint
    src/main.cpp
    src/app/App.cpp
    src/app/StringManager.cpp
    src/core/NoteMapping.cpp
    src/core/MidiEvents.cpp
    src/core/SysExEncoder.cpp
    src/hal/AdcDriver.cpp
    src/hal/Timer.cpp
    src/hal/MidiDinOut.cpp
    src/hal/LedDriver.cpp
    src/dsp/EnvelopeFollower.cpp
    src/dsp/PitchDetectorYin.cpp
    src/dsp/StringProcessor.cpp
)

# Include directories
target_include_directories(bassmint PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
)

# Link against pico libraries
target_link_libraries(bassmint
    pico_stdlib
    hardware_adc
    hardware_pwm
    hardware_uart
    hardware_dma
    hardware_irq
    hardware_timer
)

# Enable USB output, disable UART output for stdio
# (we use UART for MIDI, not debug)
pico_enable_stdio_usb(bassmint 1)
pico_enable_stdio_uart(bassmint 0)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(bassmint)

# Debug output options
option(BASSMINT_DEBUG_STATS "Enable debug statistics output" OFF)

if(BASSMINT_DEBUG_STATS)
    target_compile_definitions(bassmint PRIVATE BASSMINT_DEBUG_STATS=1)
endif()

# Compiler optimizations for embedded
target_compile_options(bassmint PRIVATE
    -Wall
    -Wextra
    -Werror=return-type
    -O2
    -ffunction-sections
    -fdata-sections
)

target_link_options(bassmint PRIVATE
    -Wl,--gc-sections
)
