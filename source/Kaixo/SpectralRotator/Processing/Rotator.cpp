
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Rotator.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    void SineSweepConvolver::trigger() {

    }

    void SineSweepConvolver::finish() {

    }

    void SineSweepConvolver::progress(float progress) {

    }

    // ------------------------------------------------

    void SineSweepConvolver::process() {

    }

    void SineSweepConvolver::prepare(double sampleRate, std::size_t maxBufferSize) {

    }

    void SineSweepConvolver::reset() {

    }

    // ------------------------------------------------

    void RingModulator::trigger() {
        m_Phase = 0;
    }

    void RingModulator::finish() {

    }

    void RingModulator::progress(float progress) {
        m_Progress = progress;
    }

    // ------------------------------------------------

    void RingModulator::process() {
        const float deltaPhase = (1 - m_Progress) * 0.5;
        const float sine = Math::nsin(m_Phase);

        output = input * sine;

        m_Phase = Math::Fast::fmod1(m_Phase + deltaPhase);
    }

    // ------------------------------------------------

}

// ------------------------------------------------
