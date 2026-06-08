
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/ProgressCounter.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    ProgressCounter::ScopedFinish::~ScopedFinish() { m_Self->done(); };
    ProgressCounter::ScopedFinish::ScopedFinish(ProgressCounter* self) : m_Self(self) { self->reset(); }

    // ------------------------------------------------

    ProgressCounter::ScopedFinish ProgressCounter::scoped() { return { this }; }

    // ------------------------------------------------

    void ProgressCounter::reset() { m_Steps = 0; m_Estimate = 1; }
    void ProgressCounter::increaseEstimate(std::int64_t v) { m_Estimate += v; }
    void ProgressCounter::step() { m_Steps++; }
    void ProgressCounter::done() { m_Steps = m_Estimate.load(); }

    // ------------------------------------------------

    float ProgressCounter::progress() const { return Math::clamp1(static_cast<float>(m_Steps) / m_Estimate); }

    // ------------------------------------------------

}

// ------------------------------------------------
