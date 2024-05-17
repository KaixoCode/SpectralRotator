
// ------------------------------------------------

#include <utility>

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Rotator.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    struct ComplexBuffer {
        std::vector<std::complex<float>> l;
        std::vector<std::complex<float>> r;

        constexpr std::size_t size() const { return l.size(); }

        constexpr void reserve(std::size_t size) {
            l.reserve(size);
            r.reserve(size);
        }

        constexpr void resize(std::size_t size) {
            l.resize(size);
            r.resize(size);
        }
    };

    // ------------------------------------------------

    constexpr void fftImpl(std::vector<std::complex<float>>& x, bool inverse = false) {
        std::vector<std::complex<float>> w(x.size(), 0.0f);

        w[0] = 1.0f;
        for (std::size_t pow2 = 1; pow2 < x.size(); pow2 *= 2) {
            w[pow2] = std::polar(1.0f, 2 * std::numbers::pi_v<float> * pow2 / x.size() * (inverse ? 1 : -1));
        }

        for (std::size_t i = 3, last = 2; i < x.size(); i++) {
            if (w[i] == 0.0f) {
                w[i] = w[last] * w[i - last];
            }
            else {
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

    constexpr void fft(ComplexBuffer& buffer) {
        fftImpl(buffer.l, false);
        fftImpl(buffer.r, false);
    }

    constexpr void ifft(ComplexBuffer& buffer) {
        fftImpl(buffer.l, true);
        fftImpl(buffer.r, true);
    }

    // ------------------------------------------------

    constexpr ComplexBuffer generateSineSweep(std::size_t size) {
        const auto nextPower2 = std::bit_ceil(size * 2);

        ComplexBuffer sweep;
        sweep.resize(nextPower2);

        float phase = 0;
        for (std::size_t i = 0; i < size; ++i) {
            float progress = static_cast<float>(i) / size;
            float deltaPhase = 0.5 * progress;
            float sine = Math::nsin(phase);

            sweep.l[i] = sine;
            sweep.r[i] = sine;

            phase = Math::Fast::fmod1(phase + deltaPhase);
        }
        
        fft(sweep);

        return sweep;
    }

    // ------------------------------------------------

    constexpr void convolve(ComplexBuffer& a, const ComplexBuffer& b) {
        const std::size_t size = Math::Fast::min(a.size(), b.size());

        for (std::size_t i = 0; i < size; ++i) a.l[i] *= b.l[i];
        for (std::size_t i = 0; i < size; ++i) a.r[i] *= b.r[i];
    }

    // ------------------------------------------------

    constexpr void hilbert(ComplexBuffer& in, bool positive) {
        const std::size_t nyquist = in.size() / 2;

        if (positive) {
            std::memset(in.l.data() + nyquist + 1, 0, (in.size() / 2 - 1) * sizeof(std::complex<float>));
            std::memset(in.r.data() + nyquist + 1, 0, (in.size() / 2 - 1) * sizeof(std::complex<float>));
        } else {
            std::memset(in.l.data() + 1, 0, (in.size() / 2 - 1) * sizeof(std::complex<float>));
            std::memset(in.r.data() + 1, 0, (in.size() / 2 - 1) * sizeof(std::complex<float>));
        }
    }

    // ------------------------------------------------

    constexpr void ringmod(ComplexBuffer& in, std::size_t originalSize) {
        float phase = 0;

        for (std::size_t i = 0; i < originalSize * 2; ++i) {
            float progress = static_cast<float>(i) / originalSize;
            float deltaPhase = Math::Fast::abs(0.5 - 0.5 * progress);
            float sine = Math::nsin(phase);
            float cose = Math::ncos(phase);

            in.l[i] = { in.l[i].real() * cose - in.l[i].imag() * sine, 0 };
            in.r[i] = { in.r[i].real() * cose - in.r[i].imag() * sine, 0 };

            phase = Math::Fast::fmod1(phase + deltaPhase);
        }
    }

    // ------------------------------------------------
    
    AudioBuffer Rotator::rotate(const AudioBuffer& buffer) {
        const auto nextPower2 = std::bit_ceil(buffer.size() * 2);

        ComplexBuffer result;
        result.resize(nextPower2);

        for (std::size_t i = 0; i < buffer.size(); ++i) {
            result.l[i] = buffer[i].l;
            result.r[i] = buffer[i].r;
        }

        ComplexBuffer sineSweep = generateSineSweep(buffer.size());

        fft(result);
        
        convolve(result, sineSweep);
        
        ComplexBuffer half1;
        half1.resize(nextPower2);
        
        std::memcpy(half1.l.data(), result.l.data(), nextPower2 * sizeof(std::complex<float>));
        std::memcpy(half1.r.data(), result.r.data(), nextPower2 * sizeof(std::complex<float>));
        
        hilbert(half1, true);
        hilbert(result, false);
        
        ifft(half1);
        ifft(result);
        
        std::memcpy(result.l.data(), half1.l.data(), buffer.size() * sizeof(std::complex<float>));
        std::memcpy(result.r.data(), half1.r.data(), buffer.size() * sizeof(std::complex<float>));
        
        ringmod(result, buffer.size());

        fft(result);
        
        convolve(result, sineSweep);
        
        ifft(result);

        AudioBuffer output;
        output.resize(buffer.size());
        
        for (std::size_t i = 0; i < buffer.size(); ++i) {
            output[i] = {
                result.l[i + buffer.size()].real(),
                result.r[i + buffer.size()].real(),
            };
        }

        return output;
    }

    // ------------------------------------------------

}

// ------------------------------------------------
