
// ------------------------------------------------

#include <utility>

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Rotator.hpp"
#include "Kaixo/SpectralRotator/Processing/Resampler.hpp"
#include "Kaixo/SpectralRotator/Processing/Fft.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    AudioBuffer Rotator::rotate(const AudioBuffer& buffer, Rotation direction, const AudioBuffer& originalBuffer) {

        // ------------------------------------------------

        if (buffer.size() == 0) {
            return {};
        }

        // ------------------------------------------------
        
        start();

        // ------------------------------------------------

        bool usingOriginal = originalBuffer.size() != 0;
        const std::size_t bufferSize = (usingOriginal ? originalBuffer.size() : buffer.size());
        const std::size_t resultSize = 2 * bufferSize - 1;

        // ------------------------------------------------

        if (direction == None) {

            // ------------------------------------------------

            return buffer;

            // ------------------------------------------------

        } else if (direction == Flip) {

            // ------------------------------------------------

            m_EstimatedSteps = bufferSize * 3 + resultSize;

            // ------------------------------------------------

            return rotateOnce(buffer, direction, originalBuffer);

            // ------------------------------------------------

        } else {

            // ------------------------------------------------

            m_EstimatedSteps = bufferSize * 3;
            if (direction == Rotate270 || direction == Rotate90) {
                m_EstimatedSteps += m_Fft.estimateSteps(resultSize, true) * 2;
            }

            // ------------------------------------------------

            return rotateOnce(buffer, direction, originalBuffer);

            // ------------------------------------------------

        }

        // ------------------------------------------------

    }

    AudioBuffer Rotator::rotateOnce(const AudioBuffer& buffer, Rotation direction, const AudioBuffer& originalBuffer) {

        // ------------------------------------------------

        bool usingOriginal = false; // originalBuffer.size() != 0;
        const std::size_t bufferSize = (usingOriginal ? originalBuffer.size() : buffer.size());
        const std::size_t resultSize = 2 * bufferSize - 1;

        // ------------------------------------------------

        bool reverseInput = direction == Rotate90 || direction == Reverse;
        bool reverseOutput = direction == Rotate270;

        // ------------------------------------------------

        ComplexBuffer result{};
        result.resize(resultSize);

        AudioBufferResampler resampler{};
        resampler.samplerate.in = buffer.sampleRate;
        resampler.samplerate.out = originalBuffer.sampleRate;
        auto get = [&](std::size_t index) {
            if (usingOriginal && buffer.sampleRate != originalBuffer.sampleRate) {
                return resampler.generate(buffer);
            } else {
                return Stereo{ buffer[index].l, buffer[index].r };
            }
        };

        Stereo sumInput{};
        for (std::size_t i = 0; i < bufferSize; ++i) {
            auto index = reverseInput ? bufferSize - i - 1 : i;
            auto out = get(i);
            result.l[index] = out.l;
            result.r[index] = out.r;

            if (i != 0) {
                result.l[resultSize - index] = out.l;
                result.r[resultSize - index] = out.r;
            }
            
            if (usingOriginal) { // When using original we want to maintain power of the original buffer
                auto level = Math::Fast::abs(Stereo{ originalBuffer[i].l, originalBuffer[i].r });
                sumInput += level * level;
            } else {
                auto level = Math::Fast::abs(out);
                sumInput += level * level;
            }

            step();
        }

        // ------------------------------------------------

        switch (direction) {
        case Rotate90:
        case Rotate270:
            ifft(result);
            break;

        case Flip: 
            // Spectral flip can be achieved by ring modulating with Nyquist
            for (std::size_t i = 0; i < resultSize; ++i) {
                result.l[i] *= i % 2 == 0 ? 1 : -1;
                result.r[i] *= i % 2 == 0 ? 1 : -1;
                step();
            }

            break;
        }

        // ------------------------------------------------

        AudioBuffer output;
        output.sampleRate = usingOriginal ? originalBuffer.sampleRate : buffer.sampleRate;
        output.resize(bufferSize);

        Stereo sumOutput{};
        for (std::size_t i = 0; i < bufferSize; ++i) {
            auto index = reverseOutput ? bufferSize - i - 1 : i;
            output[index] = {
                result.l[i].real(),
                result.r[i].real(),
            };

            auto level = Math::Fast::abs(Stereo{ output[index].l, output[index].r });
            sumOutput += level * level;

            step();
        }

        Stereo energyRatio = Math::Fast::sqrt(sumInput / sumOutput);
        for (auto& out : output) {
            out.l *= energyRatio.l;
            out.r *= energyRatio.r;
            step();
        }

        // ------------------------------------------------

        return output;

        // ------------------------------------------------
        
    }

    // ------------------------------------------------

    void Rotator::fft(ComplexBuffer& buffer) {
        m_Fft.transform(buffer.l, false);
        m_Fft.transform(buffer.r, false);
    }

    void Rotator::ifft(ComplexBuffer& buffer) {
        m_Fft.transform(buffer.l, true);
        m_Fft.transform(buffer.r, true);
    }

    // ------------------------------------------------

}

// ------------------------------------------------
