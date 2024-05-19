
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/AudioBufferSpectralInformation.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Fft.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    float AudioBufferSpectralInformation::FourierFrame::intensityAt(float x, float dx) {
        float xval = x * (intensity.size() - 2);

        std::size_t beginX = Math::max(0, xval - dx / 2);
        std::size_t endX = Math::min(intensity.size() - 1, xval + dx / 2 + 0.5);
        std::size_t range = endX - beginX;
        float max = -INFINITY;
        float min = 0;
        for (std::size_t xpos = beginX; xpos < endX; ++xpos) {
            if (max < intensity[xpos]) max = intensity[xpos];
            if (min > intensity[xpos]) min = intensity[xpos];
        }

        return (max + min) / 2;
    }

    // ------------------------------------------------

    float AudioBufferSpectralInformation::intensityAt(float x, float y, float dy) {
        if (frames.size() == 0) return 0;

        float xval = x * (frames.size() - 2);
        std::size_t x1 = xval;
        std::size_t x2 = 1 + xval;
        float xRatio = x2 - xval;

        float value1 = frames[x1].intensityAt(y, dy);
        float value2 = frames[x2].intensityAt(y, dy);

        float db = value1 * xRatio + value2 * (1 - xRatio);

        return db / 100 + 1;
    }

    // ------------------------------------------------

    AudioBufferSpectralInformation AudioBufferSpectralInformation::analyze(const Processing::AudioBuffer& buffer, std::size_t fftSize, std::size_t horizontalResolution) {
        std::size_t stepSize = Math::max(1, buffer.size() / horizontalResolution);
        std::size_t size = fftSize < buffer.size() ? buffer.size() - fftSize : buffer.size();

        auto get = [&](std::size_t index) -> AudioFrame {
            if (index < buffer.size()) return buffer[index];
            return { 0, 0 };
            };

        std::vector<std::complex<float>> fftBuffer(fftSize);

        AudioBufferSpectralInformation result;
        result.frames.reserve(horizontalResolution);
        for (std::size_t i = 0; i < size; i += stepSize) {
            auto& frame = result.frames.emplace_back();
            frame.intensity.reserve(fftSize / 2);

            for (std::size_t j = 0; j < fftSize; ++j) {
                float window = Math::nsin(0.5 * static_cast<float>(j) / fftSize);
                fftBuffer[j] = get(i + j).average() * window;
            }

            Fft{}.transform(fftBuffer, false);

            float scale = 2.f / std::numbers::pi_v<float>;
            for (std::size_t j = 0; j < fftSize / 2; ++j) {
                frame.intensity.push_back(Math::magnitude_to_db(2 * std::abs(fftBuffer[j]) / (fftSize * scale)));
            }
        }

        return result;
    }

    // ------------------------------------------------

}

// ------------------------------------------------
