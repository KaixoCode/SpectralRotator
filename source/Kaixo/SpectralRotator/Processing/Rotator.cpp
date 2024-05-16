
// ------------------------------------------------

#include <utility>

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Rotator.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    //Folowing is for SFINAE    
    template <typename T>
    struct extractType;

    template <template <typename ...> class C, typename D>
    struct extractType<C<D>> { using subType = D; };

    // Cooley–Tukey Fast Fourier Transform algorithm
    // Recursive Divide and Conquer implementation
    // Higher memory requirements and redundancy although more intuitive
    template<typename InputIt,
        typename value_type = typename std::iterator_traits<InputIt>::value_type>
    typename std::enable_if<std::is_same<value_type,
        std::complex<typename extractType<value_type>::subType>>::value,
        void>::type // Only accepts std::complex numbers containers 
        FFT2(InputIt begin, InputIt end)
    {
        typename std::iterator_traits<InputIt>::difference_type N = std::distance(begin, end);

        if (N < 2) return;
        else {
            // divide
            std::stable_partition(begin, end, [&begin](auto& a) {
                return std::distance(&*begin, &a) % 2 == 0; // pair indexes on the first half and odd on the last
                });

            //conquer
            FFT2(begin, begin + N / 2);   // recurse even items
            FFT2(begin + N / 2, end);   // recurse odd  items

            //combine
            for (decltype(N) k = 0; k < N / 2; ++k) {
                value_type even = *(begin + k);
                value_type odd = *(begin + k + N / 2);
                value_type w = std::exp(value_type(0, -2. * std::numbers::pi_v<float> * k / N)) * odd;

                *(begin + k) = even + w;
                *(begin + k + N / 2) = even - w;
            }
        }
    }

    // Inverse FFT
    template<typename InputIt,
        typename value_type = typename std::iterator_traits<InputIt>::value_type>
    typename std::enable_if<std::is_same<value_type,
        std::complex<typename extractType<value_type>::subType>>::value,
        void>::type
        IFFT2(InputIt begin, InputIt end)
    {
        typename std::iterator_traits<InputIt>::difference_type N = std::distance(begin, end);

        if (N < 2) return;
        else {
            // divide
            std::stable_partition(begin, end, [&begin](auto& a) {
                a = std::conj(a); // use the conjugate value
                return std::distance(&*begin, &a) % 2 == 0; // pair indexes on the first half and odd on the last
                });

            //conquer
            FFT2(begin, begin + N / 2);   // recurse even items on normal FFT
            FFT2(begin + N / 2, end);   // recurse odd  items on normal FFT

            //combine
            for (decltype(N) k = 0; k < N / 2; ++k) {
                value_type even = *(begin + k);
                value_type odd = *(begin + k + N / 2);
                value_type w = std::exp(value_type(0, -2. * std::numbers::pi_v<float> * k / N)) * odd;

                *(begin + k) = std::conj(even + w); //conjugate again and scale 
                *(begin + k) /= N;
                *(begin + k + N / 2) = std::conj(even - w);
                *(begin + k + N / 2) /= N;
            }
        }
    }

    // ------------------------------------------------

    struct ComplexBuffer {
        std::vector<std::complex<float>> l;
        std::vector<std::complex<float>> r;

        std::size_t size() const { return l.size(); }

        void reserve(std::size_t size) {
            l.reserve(size);
            r.reserve(size);
        }

        void resize(std::size_t size) {
            l.resize(size);
            r.resize(size);
        }
    };

    // ------------------------------------------------

    void fft(ComplexBuffer& buffer) {
        FFT2(buffer.l.begin(), buffer.l.end());
        FFT2(buffer.r.begin(), buffer.r.end());
    }

    void ifft(ComplexBuffer& buffer) {
        IFFT2(buffer.l.begin(), buffer.l.end());
        IFFT2(buffer.r.begin(), buffer.r.end());
    }

    // ------------------------------------------------

    ComplexBuffer generateSineSweep(std::size_t size) {
        const auto nextPower2 = std::bit_ceil(size * 2);

        ComplexBuffer sweep;
        sweep.resize(nextPower2);

        float phase = 0;

        for (std::size_t i = 0; i < size; ++i) {
            float deltaPhase = 0.5 * static_cast<float>(i) / size;
            float sine = Math::nsin(phase);

            sweep.l[i] = sine;
            sweep.r[i] = sine;

            phase = Math::Fast::fmod1(phase + deltaPhase);
        }
        
        for (std::size_t i = 0; i < size; ++i) {
            float deltaPhase = 0.5;
            float sine = Math::nsin(phase);

            sweep.l[i + size] = sine;
            sweep.r[i + size] = sine;

            phase = Math::Fast::fmod1(phase + deltaPhase);
        }

        fft(sweep);

        return sweep;
    }

    // ------------------------------------------------

    void convolve(ComplexBuffer& a, const ComplexBuffer& b) {
        const std::size_t size = std::min(a.size(), b.size());

        for (std::size_t i = 0; i < size; ++i) a.l[i] *= b.l[i];
        for (std::size_t i = 0; i < size; ++i) a.r[i] *= b.r[i];
    }

    // ------------------------------------------------

    void hilbert(ComplexBuffer& in, bool positive) {
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

    void ringmod(ComplexBuffer& in, std::size_t originalSize) {
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
        //output.resize(nextPower2);
        //
        //for (std::size_t i = 0; i < nextPower2; ++i) {
        //    output[i] = {
        //        result.l[i].real(),
        //        result.r[i].real(),
        //    };
        //}        
        
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
