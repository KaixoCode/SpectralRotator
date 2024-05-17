
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

    constexpr void fft(ComplexBuffer& buffer) {
        fftImpl(buffer.l, false);
        fftImpl(buffer.r, false);
    }

    constexpr void ifft(ComplexBuffer& buffer) {
        fftImpl(buffer.l, true);
        fftImpl(buffer.r, true);
    }

    // ------------------------------------------------

    /**
     * Generates a buffer containing the complex Fourier Transform of a 
     * linear sine sweep from 0Hz to Nyquist. This directly generates the
     * frequency-domain buffer without using a Fourier Transform to prevent
     * artifacts at both ends of the sine sweep. As a side-effect this means
     * the buffer only contains negative frequency content.
     * 
     * @param size number of samples to sweep over
     */
    constexpr ComplexBuffer generateSineSweep(std::size_t size) {
        const auto nextPower2 = std::bit_ceil(size * 2);

        ComplexBuffer sweep;
        sweep.resize(nextPower2); // Need power of 2 because FFT, will be zero padded

        float phase = 0;
        const float percentOffset = 2.f * size / nextPower2;
        for (std::size_t i = 0; i < nextPower2 / 2; ++i) {
            const float progress = static_cast<float>(i) / nextPower2;
            const float deltaPhase = percentOffset * progress;
            const float sine = Math::nsin(phase);
            const float cose = Math::ncos(phase);

            sweep.l[nextPower2 - i - 1] = { cose, sine };
            sweep.r[nextPower2 - i - 1] = { cose, sine };

            phase = Math::Fast::fmod1(phase + deltaPhase);
        }

        return sweep;
    }

    // ------------------------------------------------

    /**
     * Applies frequency-domain convolution of 2 frequency-domain buffers.
     * 
     * @param in input buffer
     * @param impulse frequency-domain impulse response
     */
    constexpr void convolve(ComplexBuffer& in, const ComplexBuffer& impulse) {
        const std::size_t size = Math::Fast::min(in.size(), impulse.size());

        for (std::size_t i = 0; i < size; ++i) in.l[i] *= impulse.l[i];
        for (std::size_t i = 0; i < size; ++i) in.r[i] *= impulse.r[i];
    }

    // ------------------------------------------------

    /**
     * Mirrors the positive and negative frequencies of the frequency-domain buffer.
     * 
     * @param in frequency-domain input buffer
     */
    constexpr void mirror(ComplexBuffer& in) {
        for (int k = 1; k < in.size() / 2; ++k) {
            const std::complex<float> tempL = in.l[in.size() - k];
            const std::complex<float> tempR = in.r[in.size() - k];
            in.l[in.size() - k] = std::conj(in.l[k]);
            in.r[in.size() - k] = std::conj(in.r[k]);
            in.l[k] = std::conj(tempL);
            in.r[k] = std::conj(tempR);
        }
    }

    // ------------------------------------------------

    /**
     * Applies linearly modulated ring modulation of a certain length to the buffer,
     * starting at Nyquist, going down to 0Hz, and going back up to Nyquist.
     * 
     * @param in input buffer
     * @param length amount of samples to modulate down from Nyquist to 0Hz
     */
    constexpr void ringmod(ComplexBuffer& in, std::size_t length) {
        float phase = 0;
        for (std::size_t i = 0; i < length * 2; ++i) {
            const float progress = static_cast<float>(i) / length;
            const float deltaPhase = Math::Fast::abs(0.5 - 0.5 * progress);
            const float sine = Math::nsin(phase);
            const float cose = Math::ncos(phase);
            
            in.l[i] = in.l[i].real() * cose - in.l[i].imag() * sine;
            in.r[i] = in.r[i].real() * cose - in.r[i].imag() * sine;

            phase = Math::Fast::fmod1(phase + deltaPhase);
        }
    }

    // ------------------------------------------------
    
    /**
     * Normalizes the audio buffer.
     * 
     * @param buffer input buffer
     */
    constexpr void normalize(AudioBuffer& buffer) {
        float maxLevel = 0;
        for (auto& frame : buffer) {
            auto level = Math::Fast::max(Math::Fast::abs(frame.l), Math::Fast::abs(frame.r));
            if (level > maxLevel) maxLevel = level;
        }
        
        if (maxLevel != 0) {
            for (auto& frame : buffer) {
                frame /= maxLevel;
            }
        }
    }

    // ------------------------------------------------
    
    AudioBuffer Rotator::rotate(const AudioBuffer& buffer, bool direction, std::size_t originalSize) {

        // ------------------------------------------------

        if (buffer.size() == 0) return {};

        // ------------------------------------------------

        const std::size_t bufferSize = originalSize == npos ? buffer.size() : originalSize;
        const std::size_t nextPower2 = std::bit_ceil(bufferSize * 2);

        // ------------------------------------------------
        //  Prepare the complex buffer
        // ------------------------------------------------

        ComplexBuffer result;
        result.resize(nextPower2); // Need power of 2 because FFT, will be zero-padded

        std::size_t minSize = std::min(bufferSize, buffer.size());
        if (direction) { // Forward rotation = just copy buffer
            for (std::size_t i = 0; i < minSize; ++i) {
                result.l[i] = buffer[i].l;
                result.r[i] = buffer[i].r;
            }
        } else { // Backward rotation = reverse the input buffer
            for (std::size_t i = 0; i < minSize; ++i) {
                result.l[i] = buffer[minSize - i - 1].l;
                result.r[i] = buffer[minSize - i - 1].r;
            }
        }

        // ------------------------------------------------
        //  Skew 1: Convolution with sine sweep
        // ------------------------------------------------

        fft(result); 
        // Result now contains frequency-domain data
        
        // sineSweep contains a frequency-domain linear sine sweep with 
        // only negative frequencies (this is a side-effect of how it is generated).
        const ComplexBuffer sineSweep = generateSineSweep(bufferSize);
        convolve(result, sineSweep); // Apply the first skew
        // Result now contains frequency-domain data with only negative frequencies.

        // ------------------------------------------------
        //  Skew 2: Linearly modulated frequency shift
        // ------------------------------------------------

        // Frequency shifting can be achieved using ring modulation. 
        // However, this generates both positive and negative frequency shifted signals.
        // This can be circumvented by removing either the positive or the negative frequencies
        // from the frequency-domain signal.
        // 
        // Since we need both positive and negative frequency shifts, the first half of 
        // the audio needs to be shifted up, and the second half needs to be shifted down,
        // we need to create both a signal with only positive, and one with only negative frequencies.
        //
        // Therefor we have to create a copy of the signal, so we can swap
        // the positive frequencies with the negative frequencies.
        ComplexBuffer mirrored;
        mirrored.resize(nextPower2);
        std::memcpy(mirrored.l.data(), result.l.data(), nextPower2 * sizeof(std::complex<float>));
        std::memcpy(mirrored.r.data(), result.r.data(), nextPower2 * sizeof(std::complex<float>));
        mirror(mirrored); 
        // Since our buffer only contained negative frequencies after the convolution,
        // mirroring the frequencies here means our mirrored buffer now only contains positive frequencies.
        
        // Now that we have both a buffer with positive frequences, and a buffer with negative frequencies
        // we need to copy over our mirrored buffer to the first half of the original buffer in the time-domain.
        ifft(mirrored); // Convert both buffers back to the time-domain
        ifft(result);
        std::memcpy(result.l.data(), mirrored.l.data(), bufferSize * sizeof(std::complex<float>));
        std::memcpy(result.r.data(), mirrored.r.data(), bufferSize * sizeof(std::complex<float>));
        // After this copy, the first half of our result buffer now contains time-domain audio data with
        // only positive frequencies, and the second half only contains negative frequencies.
        
        // Because the phase difference between positive and negative frequencies is exactly 180 degrees
        // there is now a sudden jump (a click) right where we merged the buffers, we can get rid of this jump
        // by inverting the phase of the first half of our buffer.
        for (std::size_t i = 0; i < bufferSize; ++i) {
            result.l[i] *= -1;
            result.r[i] *= -1;
        }

        // Now that our buffer contains exactly what we need, we can apply the ring modulation.
        // Instead of shifting both up and down, this will now shift down in the first half, and
        // up in the second half of the buffer.
        ringmod(result, bufferSize);
        // The resulting buffer contains frequency-shifted time-domain audio data.

        // ------------------------------------------------
        //  Skew 3: Same convolution as skew 1
        // ------------------------------------------------

        // We're currently in the time-domain, so switch back to frequency-domain for the convolution
        fft(result); 
        convolve(result, sineSweep);
        ifft(result);
        // Finally, after the 3 skews, our result buffer now contains time-domain audio data that 
        // is a 90 degrees spectral rotation of the original buffer.

        // ------------------------------------------------
        //  Copy the complex buffer to a normalized audio buffer
        // ------------------------------------------------

        AudioBuffer output;
        output.sampleRate = buffer.sampleRate;
        output.resize(bufferSize);
        
        // The part of the buffer we need has shifted exactly the length of our original
        // input buffer, and has the exact same length as well, so we extract it by
        // taking all samples from indices 'bufferSize' to '2 * bufferSize'.
        if (direction) { // Forward rotation
            for (std::size_t i = 0; i < bufferSize; ++i) {
                output[i] = {
                    result.l[i + bufferSize].real(),
                    result.r[i + bufferSize].real(),
                };
            }
        } else { // Backward rotation, means the result needs to be reversed
            for (std::size_t i = 0; i < bufferSize; ++i) {
                output[bufferSize - i - 1] = {
                    result.l[i + bufferSize].real(),
                    result.r[i + bufferSize].real(),
                };
            }
        }

        normalize(output);

        // ------------------------------------------------

        return output;

        // ------------------------------------------------

    }

    // ------------------------------------------------

}

// ------------------------------------------------
