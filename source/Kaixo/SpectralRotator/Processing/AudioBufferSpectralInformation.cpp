
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/AudioBufferSpectralInformation.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Fft.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    

    // ------------------------------------------------
    
    float AudioBufferSpectralInformation::get(std::size_t x, std::size_t y) {
        std::size_t index = x * frameSize + y;
        if (index < intensity.size()) return intensity[index];
        else return 0;
    }

    // ------------------------------------------------

    float AudioBufferSpectralInformation::intensityAtY(std::size_t x, float y, float dy) {
        float yval = y * (frameSize - 2);
        if (dy < 1) {
            std::size_t beginY = yval;
            std::size_t endY = yval + 1;
            float ratio = yval - beginY;
            ratio *= ratio * ratio;
            return (get(x, beginY) * (1 - ratio) + get(x, endY) * ratio);
        }

        std::size_t beginY = Math::max(0, yval - dy / 2);
        std::size_t endY = Math::min(frameSize - 1, yval + dy / 2 + 1);
        std::size_t range = endY - beginY;

        float scale = 0;
        float sum = 0;
        for (std::size_t ypos = beginY; ypos <= endY; ++ypos) {
            float window = 0.5 * (1 - Math::Fast::ncos(0.5 * static_cast<float>(ypos - beginY) / (endY - beginY)));
            sum += get(x, ypos) * window;
            scale += window;
        }

        return sum / scale;
    }

    // ------------------------------------------------

    float AudioBufferSpectralInformation::intensityAt(float x, float dx, float y, float dy) {
        if (intensity.size() == 0) return 0;

        float xval = x * (frames() - 2);
        if (dx < 1) {
            std::size_t beginX = xval;
            std::size_t endX = xval + 1;
            float ratio = xval - beginX;
            ratio *= ratio * ratio;
            return (intensityAtY(beginX, y, dy) * (1 - ratio) + intensityAtY(endX, y, dy) * ratio);
        }

        std::size_t beginX = Math::max(0, xval - dx / 2);
        std::size_t endX = Math::min(frames() - 1, xval + dx / 2 + 1);
        std::size_t range = endX - beginX;

        float scale = 0;
        float sum = 0;
        for (std::size_t xpos = beginX; xpos <= endX; ++xpos) {
            float window = 0.5 * (1 - Math::Fast::ncos(0.5 * static_cast<float>(xpos - beginX) / (endX - beginX)));
            sum += intensityAtY(xpos, y, dy) * window;
            scale += window;
        }

        return sum / scale;
    }

    // ------------------------------------------------

    AudioBufferSpectralInformation AudioBufferSpectralInformation::analyze(
        const Processing::AudioBuffer& buffer, std::size_t fftSize, 
        std::size_t horizontalResolution, std::size_t* progress) 
    {
        if (buffer.empty()) return {};

        std::size_t stepSize = Math::max(1, buffer.size() / horizontalResolution);
        std::size_t size = buffer.size();
        std::size_t blockSize = Math::min(fftSize, buffer.sampleRate * 0.05); // 50 millis

        auto get = [&](std::size_t index) -> AudioFrame {
            if (index < buffer.size()) return buffer[index];
            return { 0, 0 };
        };

        std::vector<std::complex<float>> fftBuffer(fftSize);

        AudioBufferSpectralInformation result;
        result.frameSize = fftSize / 2 + 1;
        result.intensity.resize(result.frameSize * horizontalResolution);
        Fft fft;
        fft.stepRef = progress;
        for (std::size_t x = 0; x < horizontalResolution; ++x) {
            std::size_t i = x * stepSize;

            std::memset(fftBuffer.data(), 0, fftSize * sizeof(std::complex<float>));
            float scale = 0;
            for (std::size_t j = 0; j < blockSize; ++j) {
                float progress = static_cast<float>(j) / (blockSize - 1);
                float window = 0.5f * (1.0f - Math::Fast::ncos(progress));
                scale += window;
                fftBuffer[j] = get(i + j).average() * window;
            }

            fft.transform(fftBuffer, false);

            for (std::size_t y = 0; y < fftSize / 2 + 1; ++y) {
                std::size_t index = y + x * result.frameSize;
                float magnitude = std::abs(fftBuffer[y]);
                float normalizedMagnitude = (2 * magnitude) / scale; // Apply proper scaling
                result.intensity[index] = Math::Fast::magnitude_to_db(normalizedMagnitude);
                if (result.intensity[index] == -INFINITY) result.intensity[index] = -145;
            }
        }

        return result;
    }

    // ------------------------------------------------

}

// ------------------------------------------------
