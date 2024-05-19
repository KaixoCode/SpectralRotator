
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Resampler.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    Stereo AudioBufferResampler::generate(const AudioBuffer& in) {

        // ------------------------------------------------

        Stereo result;

        // ------------------------------------------------

        const double delta = samplerate.in / samplerate.out;

        // ------------------------------------------------
        
        const auto get = [&](std::int64_t index) -> AudioFrame {
            if (index >= 0 && index < in.size()) return reverse ? in[in.size() - index - 1] : in[index];
            finished = true;
            return {};
        };

        // ------------------------------------------------

        if (delta == 1) {

            // ------------------------------------------------

            auto [bl, br] = get(lastinserted);
            buffer[lastinserted] = { bl, br };
            result.l = bl;
            result.r = br;
            lastinserted++;

            // ------------------------------------------------

        } else if (delta < 1) {

            // ------------------------------------------------

            const std::int64_t offset = static_cast<std::int64_t>(playindex + (WINDOW_SIZE / 2) - 1);
            for (std::int64_t i = lastinserted; i < offset; ++i) {
                auto [bl, br] = get(i);
                buffer[i] = { bl, br };
            }
            lastinserted = offset;

            // ------------------------------------------------

            applyWindowedSinc(result);

            // ------------------------------------------------

            aafp[0].type = FilterType::LowPass;
            aafp[0].sampleRate = samplerate.out;
            aafp[0].f0 = samplerate.in * 0.499; // << rounding correction
            aafp[0].recalculateParameters();

            result = aaf[0].process(result, aafp[0]);

            // ------------------------------------------------

        } else {

            // ------------------------------------------------

            aafp[1].type = FilterType::LowPass;
            aafp[1].sampleRate = samplerate.in;
            aafp[1].f0 = samplerate.out * 0.499; // << rounding correction
            aafp[1].recalculateParameters();

            const std::int64_t offset = static_cast<std::int64_t>(playindex + (WINDOW_SIZE / 2) - 1);
            for (std::int64_t i = lastinserted; i < offset; ++i) {
                auto [bl, br] = get(i);
                buffer[i] = aaf[1].process({ bl, br }, aafp[1]);
            }

            lastinserted = offset;

            // ------------------------------------------------

            applyWindowedSinc(result);

            // ------------------------------------------------

        }

        // ------------------------------------------------

        playindex += delta;
        return result;
    }

    // ------------------------------------------------

    void AudioBufferResampler::applyWindowedSinc(Stereo& in) {
        for (std::int64_t i = -WINDOW_SIZE / 2; i < (WINDOW_SIZE / 2) - 1; ++i) {
            const std::int64_t j = static_cast<std::int64_t>(playindex + i);
            if (j >= 0) {
                constexpr double pi = std::numbers::pi_v<double>;
                const double w = j - playindex;
                const double window = 0.5 - 0.5 * Math::Fast::cos(2 * pi * (0.5 + w / WINDOW_SIZE));
                const double alpha = pi * w;
                const double sinc = alpha == 0 ? 1 : Math::Fast::sin(alpha) / alpha;
                in += window * sinc * buffer[j];
            }
        }
    }

    // ------------------------------------------------

}

// ------------------------------------------------
