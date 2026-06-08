#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Utils/ReadWriteLock.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {
    
    // ------------------------------------------------

    /**
        Wrapper around a juce audio buffer that safely allows read/write.
     */
    class SafeAudioBuffer {
    public:

        // ------------------------------------------------

        class ReadBuffer {
        public:

            // ------------------------------------------------

            ReadBuffer(const juce::AudioBuffer<float>& bfr, float sampleRate, std::int64_t startOffset);

            // ------------------------------------------------

            Stereo operator[](std::int64_t index) const;

            // ------------------------------------------------
            
            /** Get the size of the buffer in samples. Returns 0 if currently writing.
        
                @returns the size of the buffer.
             */
            std::size_t size() const;

            /** Get the sample rate of the buffer. Returns 0 if currently writing.
            
                @returns the sample rate of the buffer.
             */
            float sampleRate() const;

            // ------------------------------------------------

        private:
            const juce::AudioBuffer<float>& m_Buffer;
            float m_SampleRate;
            std::int64_t m_StartOffset;

            // ------------------------------------------------

        };

        // ------------------------------------------------

        using Buffer = juce::AudioBuffer<float>;
        using Callback = std::function<void(Buffer& buffer, float& sampleRate, std::int64_t& startOffset)>;
        using ConstCallback = std::function<void(ReadBuffer buffer)>;

        // ------------------------------------------------

        /** Access the buffer for writing, from a callback.
            
            @param callback             callback that will be given the buffer for writing.
         */
        void access(Callback callback);

        /** Access the buffer for reading, from a callback.
            
            @param callback             callback that will be given the buffer for writing.
         */
        void access(ConstCallback callback) const;

        // ------------------------------------------------
        
        /** Get the size of the buffer in samples. Returns 0 if currently writing.
        
            @returns the size of the buffer.
         */
        std::size_t size() const;

        /** Get the sample rate of the buffer. Returns 0 if currently writing.
            
            @returns the sample rate of the buffer.
         */
        float sampleRate() const;

        // ------------------------------------------------

        /** Read the sample at index. Handles bounds checking. Results in silence if currently writing.

            @param index                index in the buffer.

            @returns the sample in the buffer at the index.
         */
        Stereo read(std::int64_t index) const;

        // ------------------------------------------------

        // Used to offset the buffer while reading.
        std::atomic_int64_t startOffset = 0;

        // ------------------------------------------------

    private:
        juce::AudioBuffer<float> m_Buffer{};
        float m_SampleRate = 44100.0f;
        ReadWriteLock m_Lock{};
    };

    // ------------------------------------------------

}

// ------------------------------------------------
