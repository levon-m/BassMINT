#pragma once

#include <cstdint>
#include <cstring>
#include <array>

namespace BassMINT {

/**
 * @brief Lock-free single-producer single-consumer ring buffer
 *
 * Template-based, fixed-size ring buffer for audio samples.
 * - Producer: ISR context (ADC callback)
 * - Consumer: Main loop (DSP processing)
 * - SIZE must be power of 2 for efficient masking
 *
 * @tparam T Sample type (typically uint16_t or float)
 * @tparam SIZE Buffer size (must be power of 2)
 */
template<typename T, size_t SIZE>
class RingBuffer {
    static_assert((SIZE & (SIZE - 1)) == 0, "SIZE must be power of 2");

public:
    RingBuffer() : writeIndex_(0), readIndex_(0) {
        // Zero-initialize buffer
        std::memset(buffer_.data(), 0, sizeof(buffer_));
    }

    /**
     * @brief Push a single sample (ISR context)
     * @param sample Sample to push
     * @return true if pushed, false if buffer full
     */
    bool push(T sample) {
        uint32_t nextWrite = (writeIndex_ + 1) & MASK;

        // Check if full (write would catch read)
        if (nextWrite == readIndex_) {
            return false; // Buffer full
        }

        buffer_[writeIndex_] = sample;
        writeIndex_ = nextWrite;
        return true;
    }

    /**
     * @brief Pop a single sample (main loop context)
     * @param sample Output parameter for popped sample
     * @return true if popped, false if buffer empty
     */
    bool pop(T& sample) {
        // Check if empty
        if (readIndex_ == writeIndex_) {
            return false;
        }

        sample = buffer_[readIndex_];
        readIndex_ = (readIndex_ + 1) & MASK;
        return true;
    }

    /**
     * @brief Peek at available samples without removing
     * @param output Output array (must have space for 'count')
     * @param count Number of samples to peek
     * @return Actual number of samples peeked
     */
    size_t peek(T* output, size_t count) const {
        size_t available = getAvailable();
        size_t toPeek = (count < available) ? count : available;

        for (size_t i = 0; i < toPeek; ++i) {
            output[i] = buffer_[(readIndex_ + i) & MASK];
        }

        return toPeek;
    }

    /**
     * @brief Read block of samples (main loop context)
     * @param output Output array (must have space for 'count')
     * @param count Number of samples to read
     * @return Actual number of samples read
     */
    size_t read(T* output, size_t count) {
        size_t available = getAvailable();
        size_t toRead = (count < available) ? count : available;

        for (size_t i = 0; i < toRead; ++i) {
            output[i] = buffer_[readIndex_];
            readIndex_ = (readIndex_ + 1) & MASK;
        }

        return toRead;
    }

    /**
     * @brief Get number of samples available to read
     */
    size_t getAvailable() const {
        return (writeIndex_ - readIndex_) & MASK;
    }

    /**
     * @brief Get free space for writing
     */
    size_t getFree() const {
        return SIZE - getAvailable() - 1; // -1 to avoid full/empty ambiguity
    }

    /**
     * @brief Check if buffer is empty
     */
    bool isEmpty() const {
        return readIndex_ == writeIndex_;
    }

    /**
     * @brief Clear the buffer
     */
    void clear() {
        readIndex_ = writeIndex_; // Just advance read to write
    }

    /**
     * @brief Get buffer capacity
     */
    static constexpr size_t capacity() { return SIZE - 1; }

private:
    static constexpr uint32_t MASK = SIZE - 1;

    std::array<T, SIZE> buffer_;
    volatile uint32_t writeIndex_; // Modified by ISR
    volatile uint32_t readIndex_;  // Modified by main loop
};

} // namespace BassMINT
