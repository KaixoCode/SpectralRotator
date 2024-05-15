
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Processing/Module.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    class SineSweepConvolver : public Module {
    public:

        // ------------------------------------------------

        void trigger(); // Triggers the convolver
        void finish(); // Signals that the sweep is done 

        void progress(float progress); // Sets progress in sweep

        // ------------------------------------------------

        bool active() const override { return true; }
        void process() override;
        void prepare(double sampleRate, std::size_t maxBufferSize) override;
        void reset() override;

        // ------------------------------------------------
        
    private:
        std::vector<Stereo> m_Buffer;

        // ------------------------------------------------

    };

    // ------------------------------------------------
    
    class RingModulator : public Module {
    public:

        // ------------------------------------------------
        
        Stereo input;
        Stereo output;

        // ------------------------------------------------

        void trigger();
        void finish();

        void progress(float progress);

        // ------------------------------------------------

        bool active() const override { return true; }
        void process() override;
        
        // ------------------------------------------------
        
    private:
        float m_Progress = 0;
        float m_Phase = 0;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
