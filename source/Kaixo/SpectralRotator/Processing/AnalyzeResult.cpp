
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/AnalyzeResult.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    float AnalyzeResult::intensityAt(float millis, float normalizedFrequency) {
        const float fftMillis = Convert::samplesToMillis(settings.fftSize, sampleRate);
        const float block = (millis - fftMillis / 2) / settings.fftResolution;
        const float bin = normalizedFrequency * (settings.fftSize / 2);
        const std::int64_t nofBlocks = static_cast<std::int64_t>(blocks.size());

        const std::int64_t block1 = static_cast<std::int64_t>(block);
        const std::int64_t block2 = block1 + 1;
        const float blockRatio = block - block1;

        const std::int64_t bin1 = static_cast<std::int64_t>(bin);
        const std::int64_t bin2 = bin1 + 1;
        const float binRatio = bin - bin1;

        float intensity1 = -144, intensity2 = -144;

        if (block1 >= 0 && block1 < nofBlocks) {
            float intensity11 = -144, intensity12 = -144;

            const auto& block1data = blocks[block1];
            const std::int64_t block1size = static_cast<std::int64_t>(block1data.result.size());

            if (bin1 >= 0 && bin1 < block1size) intensity11 = block1data.result[bin1];
            if (bin2 >= 0 && bin2 < block1size) intensity12 = block1data.result[bin2];

            intensity1 = Math::lerp(binRatio, intensity11, intensity12);
        }

        if (block2 >= 0 && block2 < nofBlocks) {
            float intensity21 = -144, intensity22 = -144;

            const auto& block2data = blocks[block2];
            const std::int64_t block2size = static_cast<std::int64_t>(block2data.result.size());

            if (bin1 >= 0 && bin1 < block2size) intensity21 = block2data.result[bin1];
            if (bin2 >= 0 && bin2 < block2size) intensity22 = block2data.result[bin2];

            intensity2 = Math::lerp(binRatio, intensity21, intensity22);
        }

        return Math::lerp(blockRatio, intensity1, intensity2) / settings.fftRange + 1;
    }

    // ------------------------------------------------

}

// ------------------------------------------------
