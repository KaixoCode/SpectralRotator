
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

        static AudioBuffer rotate(const AudioBuffer& buffer, bool direction, std::size_t originalSize = npos);

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
