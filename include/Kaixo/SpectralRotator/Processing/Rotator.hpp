
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Processing/Module.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/AudioBuffer.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    /**
     * Steps for rotating spectrum 90 degrees:
     *  - Generate an FFT'ed sine sweep of same length as input buffer
     *  - FFT the input buffer
     *  - Convolve FFT'ed input buffer with FFT'ed sine sweep
     *  - Apply Hilbert filter (remove negative frequencies)
     *  - IFFT
     *  - Apply ringmod to do a frequency shift
     *  - FFT
     *  - Convolve with the FFT'ed sine sweep again
     *  - IFFT
     *  - Convert resulting complex data back into audio samples
     */
    class Rotator {
    public:

        // ------------------------------------------------

        static AudioBuffer rotate(const AudioBuffer& buffer, bool direction, const AudioBuffer& originalBuffer = {});

        // ------------------------------------------------

    };

    // ------------------------------------------------

    constexpr void fftImpl(std::vector<std::complex<float>>& x, bool inverse = false) {
        std::vector<std::complex<float>> w(x.size(), 0.0f);

        w[0] = 1.0f;
        for (std::size_t pow2 = 1; pow2 < x.size(); pow2 *= 2) {
            w[pow2] = std::polar(1.0f, 2 * std::numbers::pi_v<float> *pow2 / x.size() * (inverse ? 1 : -1));
        }

        for (std::size_t i = 3, last = 2; i < x.size(); i++) {
            if (w[i] == 0.0f) {
                w[i] = w[last] * w[i - last];
            } else {
                last = i;
            }
        }

        std::vector<std::complex<float>> newX(x.size());
        for (std::size_t blockSize = x.size(); blockSize > 1; blockSize /= 2) {
            for (std::size_t start = 0; start < x.size(); start += blockSize) {
                for (std::size_t i = 0; i < blockSize; i++) {
                    newX[start + blockSize / 2 * (i % 2) + i / 2] = x[start + i];
                }
            }
            x = newX;
        }

        for (std::size_t blockSize = 2; blockSize <= x.size(); blockSize *= 2) {
            std::size_t wBaseI = x.size() / blockSize;
            for (std::size_t start = 0; start < x.size(); start += blockSize) {
                for (std::size_t i = 0; i < blockSize / 2; i++) {
                    newX[start + i] = x[start + i] + w[wBaseI * i] * x[start + blockSize / 2 + i];
                    newX[start + blockSize / 2 + i] = x[start + i] - w[wBaseI * i] * x[start + blockSize / 2 + i];
                }
            }
            x = newX;
        }
    }
    // ------------------------------------------------

}

// ------------------------------------------------
