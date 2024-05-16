
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Processing/Module.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    struct alignas(8) AudioFrame {

        // ------------------------------------------------

        float l = 0;
        float r = 0;

        // ------------------------------------------------

        constexpr AudioFrame & operator=(float other) { l = other, r = other; return *this; }
        constexpr AudioFrame & operator+=(const AudioFrame& other) { l += other.l, r += other.r; return *this; }
        constexpr AudioFrame & operator-=(const AudioFrame& other) { l -= other.l, r -= other.r; return *this; }
        constexpr AudioFrame & operator*=(const AudioFrame& other) { l *= other.l, r *= other.r; return *this; }
        constexpr AudioFrame & operator/=(const AudioFrame& other) { l /= other.l, r /= other.r; return *this; }
        constexpr AudioFrame & operator+=(float other) { l += other, r += other; return *this; }
        constexpr AudioFrame & operator-=(float other) { l -= other, r -= other; return *this; }
        constexpr AudioFrame & operator*=(float other) { l *= other, r *= other; return *this; }
        constexpr AudioFrame & operator/=(float other) { l /= other, r /= other; return *this; }
        constexpr friend AudioFrame operator-(const AudioFrame& a) { return { -a.l, -a.r }; }
        constexpr friend AudioFrame operator+(const AudioFrame& a, const AudioFrame& b) { return { a.l + b.l, a.r + b.r }; }
        constexpr friend AudioFrame operator-(const AudioFrame& a, const AudioFrame& b) { return { a.l - b.l, a.r - b.r }; }
        constexpr friend AudioFrame operator*(const AudioFrame& a, const AudioFrame& b) { return { a.l * b.l, a.r * b.r }; }
        constexpr friend AudioFrame operator/(const AudioFrame& a, const AudioFrame& b) { return { a.l / b.l, a.r / b.r }; }
        constexpr friend AudioFrame operator+(const AudioFrame& a, float b) { return { a.l + b, a.r + b }; }
        constexpr friend AudioFrame operator-(const AudioFrame& a, float b) { return { a.l - b, a.r - b }; }
        constexpr friend AudioFrame operator*(const AudioFrame& a, float b) { return { a.l * b, a.r * b }; }
        constexpr friend AudioFrame operator/(const AudioFrame& a, float b) { return { a.l / b, a.r / b }; }
        constexpr friend AudioFrame operator+(float a, const AudioFrame& b) { return { a + b.l, a + b.r }; }
        constexpr friend AudioFrame operator-(float a, const AudioFrame& b) { return { a - b.l, a - b.r }; }
        constexpr friend AudioFrame operator*(float a, const AudioFrame& b) { return { a * b.l, a * b.r }; }
        constexpr friend AudioFrame operator/(float a, const AudioFrame& b) { return { a / b.l, a / b.r }; }
        constexpr friend bool operator==(const AudioFrame& a, const AudioFrame& b) { return a.l == b.l && a.r == b.r; }
        constexpr friend bool operator!=(const AudioFrame& a, const AudioFrame& b) { return a.l != b.l && a.r != b.r; }
        constexpr friend bool operator==(const AudioFrame& a, float b) { return a.l == b && a.r == b; }
        constexpr friend bool operator!=(const AudioFrame& a, float b) { return a.l != b && a.r != b; }
        constexpr friend bool operator==(float a, const AudioFrame& b) { return a == b.l && a == b.r; }
        constexpr friend bool operator!=(float a, const AudioFrame& b) { return a != b.l && a != b.r; }
        constexpr float& operator[](bool value) { return value ? r : l; }
        constexpr const float& operator[](bool value) const { return value ? r : l; }
        constexpr float sum() const noexcept { return r + l; }
        constexpr float average() const noexcept { return sum() / 2; }
    };

    // ------------------------------------------------
    
    using AudioBuffer = std::vector<AudioFrame>;

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

        static AudioBuffer rotate(const AudioBuffer& buffer);

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
