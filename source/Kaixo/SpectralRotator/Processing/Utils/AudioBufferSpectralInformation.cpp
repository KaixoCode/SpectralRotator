
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

        for (auto& [id, layer] : std::views::reverse(layers)) {
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

    void AudioBufferSpectralInformation::analyze(AnalyzeSettings settings) {

        // ------------------------------------------------

        std::int64_t size = settings.buffer.size();
        std::int64_t blockSize = Math::min(settings.fftSize, settings.buffer.sampleRate * (settings.blockSize / 1000.f));
        std::int64_t distanceBetweenBlocks = Math::max((settings.horizontalResolution / 1000.f) * settings.buffer.sampleRate, 1);
        std::int64_t blocks = size / distanceBetweenBlocks;

        // ------------------------------------------------

        auto get = [&](std::int64_t index) -> AudioFrame {
            if (index >= 0 && index < settings.buffer.size()) return settings.buffer[index];
            return { 0, 0 };
        };

        // ------------------------------------------------

        std::vector<std::complex<float>> fftBuffer(settings.fftSize);

        // ------------------------------------------------

        auto& result = settings.reanalyze;
        result.frameSize = settings.fftSize / 2 + 1;
        result.intensity.resize(result.frameSize * blocks, -145);
        result.offset = { 0.f, 0.f };
        result.selection = { 0.f, 0.f, settings.buffer.size() / settings.buffer.sampleRate, settings.buffer.sampleRate / 2 };
        // ^^ Default selection is entire buffer
        result.sampleRate = settings.buffer.sampleRate;

        // ------------------------------------------------

        Fft fft;
        fft.stepRef = settings.progress;

        // ------------------------------------------------
        
        std::int64_t beginSample = settings.start * settings.buffer.sampleRate;
        std::int64_t endSample = settings.end * settings.buffer.sampleRate;

        std::int64_t beginBlock = Math::clamp(beginSample / distanceBetweenBlocks, 0, blocks);
        std::int64_t endBlock = Math::clamp(endSample / distanceBetweenBlocks, beginBlock, blocks);

        // ------------------------------------------------

        for (std::int64_t x = beginBlock; x < endBlock; ++x) {

            // ------------------------------------------------

            std::int64_t i = x * distanceBetweenBlocks;

            // ------------------------------------------------

            std::memset(fftBuffer.data(), 0, settings.fftSize * sizeof(std::complex<float>));
            float scale = 0;
            for (std::int64_t j = 0; j < blockSize; ++j) {
                float progress = static_cast<float>(j) / (blockSize - 1);
                float window = 0.5f * (1.0f - Math::Fast::ncos(progress));
                scale += window;
                fftBuffer[j] = get(i + j).average() * window;
            }

            // ------------------------------------------------

            fft.transform(fftBuffer, false);

            // ------------------------------------------------

            for (std::size_t y = 0; y < settings.fftSize / 2 + 1; ++y) {
                std::size_t index = y + x * result.frameSize;
                float magnitude = std::abs(fftBuffer[y]);
                float normalizedMagnitude = (2 * magnitude) / scale; // Apply proper scaling
                result.intensity[index] = Math::Fast::magnitude_to_db(normalizedMagnitude);
                if (result.intensity[index] < -145) result.intensity[index] = -145;
            }

            // ------------------------------------------------

        }

        // ------------------------------------------------

    }

    // ------------------------------------------------

}

// ------------------------------------------------
