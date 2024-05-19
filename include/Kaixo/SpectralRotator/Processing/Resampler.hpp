
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Processing/Module.hpp"
#include "Kaixo/Core/Processing/Filter.hpp"
#include "Kaixo/SpectralRotator/Processing/AudioBuffer.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    struct AudioBufferResampler {

        // ------------------------------------------------

        constexpr static std::int64_t WINDOW_SIZE = 64; // Windowed-Sinc Function window size.

        // ------------------------------------------------

        void reset() { index(0); }
        void index(std::int64_t i) { playindex = i; lastinserted = i; finished = false; }

        // ------------------------------------------------
        
        bool reverse = false;

        // ------------------------------------------------

        struct {
            double in;
            double out;
        } samplerate;

        // ------------------------------------------------

        Stereo generate(const AudioBuffer& buffer);

        // ------------------------------------------------
        
        std::int64_t position() const { return lastinserted; }
        bool eof() const { return finished; }

        // ------------------------------------------------

    private:
        std::int64_t lastinserted = 0;
        double playindex = 0; // Current non-integer index in input buffer.
        bool finished = false;

        // ------------------------------------------------

        struct CircleBuffer {
            Stereo data[WINDOW_SIZE]{};
            constexpr Stereo& operator[](std::int64_t i) { return data[(i + 10 * WINDOW_SIZE) % WINDOW_SIZE]; }
        } buffer;

        EllipticFilter aaf[2]{};
        EllipticParameters aafp[2]{};

        // ------------------------------------------------

        void applyWindowedSinc(Stereo& in);

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
