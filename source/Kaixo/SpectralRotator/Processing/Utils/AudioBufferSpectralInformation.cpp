
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Utils/AudioBufferSpectralInformation.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Utils/Fft.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    

    // ------------------------------------------------
    
    float AudioBufferSpectralInformation::Layer::get(std::int64_t x, std::int64_t y) {
        if (x < 0) return -145;
        std::int64_t index = x * frameSize + y;
        if (index < intensity.size()) return intensity[index];
        else return -145;
    }

    // ------------------------------------------------

    float AudioBufferSpectralInformation::Layer::intensityAtY(std::int64_t x, float y) {
        float yval = y * (frameSize - 2);
        std::int64_t beginY = yval;
        std::int64_t endY = yval + 1;
        float ratio = yval - beginY;
        return (get(x, beginY) * (1 - ratio) + get(x, endY) * ratio);
    }

    // ------------------------------------------------

    float AudioBufferSpectralInformation::Layer::intensityAt(float x, float y) {
        if (intensity.size() == 0) return -145;

        float xval = x * (frames() - 2);
        std::int64_t beginX = xval;
        std::int64_t endX = xval + 1;
        float ratio = xval - beginX;
        return (intensityAtY(beginX, y) * (1 - ratio) + intensityAtY(endX, y) * ratio);
    }

    // ------------------------------------------------

    float AudioBufferSpectralInformation::intensityAt(float x, float y) {
        float intensity = -145;

        for (auto& layer : std::views::reverse(layers)) {
            if (x > layer.selection.x() && y > layer.selection.y() &&
                x < layer.selection.x() + layer.selection.width() &&
                y < layer.selection.y() + layer.selection.height()) 
            {
                float normX = (x - layer.offset.x()) / layer.selection.width();
                float normY = 2 * (y - layer.offset.y()) / layer.sampleRate;
                intensity = layer.intensityAt(normX, normY);
                break;
            }
        }

        return intensity;
    }

    // ------------------------------------------------

    AudioBufferSpectralInformation AudioBufferSpectralInformation::analyze(
        const Processing::AudioBuffer& buffer, std::size_t fftSize, 
        float horizontalResolution, std::size_t bSizeMs, std::size_t* progress) 
    {
        if (buffer.empty()) return {};

        std::size_t size = buffer.size();
        std::size_t blockSize = Math::min(fftSize, buffer.sampleRate * (bSizeMs / 1000.f));
        std::size_t distanceBetweenBlocks = Math::clamp((horizontalResolution / 1000.f) * buffer.sampleRate, 1, size);
        std::size_t blocks = size / distanceBetweenBlocks;

        auto get = [&](std::int64_t index) -> AudioFrame {
            if (index >= 0 && index < buffer.size()) return buffer[index];
            return { 0, 0 };
        };

        std::vector<std::complex<float>> fftBuffer(fftSize);

        AudioBufferSpectralInformation spectralInformation;
        auto& result = spectralInformation.layers.emplace_back();
        result.frameSize = fftSize / 2 + 1;
        result.intensity.resize(result.frameSize * blocks);
        result.offset = { 0.f, 0.f };
        result.selection = { 0.f, 0.f, buffer.size() / buffer.sampleRate, buffer.sampleRate / 2 };
        result.sampleRate = buffer.sampleRate;
        // ^^ Default selection is entire buffer
        Fft fft;
        fft.stepRef = progress;
        for (std::int64_t x = 0; x < blocks; ++x) {
            std::int64_t i = x * distanceBetweenBlocks;

            std::memset(fftBuffer.data(), 0, fftSize * sizeof(std::complex<float>));
            float scale = 0;
            for (std::int64_t j = 0; j < blockSize; ++j) {
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
                if (result.intensity[index] < -145) result.intensity[index] = -145;
            }
        }

        return spectralInformation;
    }

    // ------------------------------------------------

}

// ------------------------------------------------
