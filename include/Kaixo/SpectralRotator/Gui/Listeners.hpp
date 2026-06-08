#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/Listeners.hpp"
#include "Kaixo/Core/Gui/View.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Interfaces/FileInterface.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    using namespace std::literals;

    // ------------------------------------------------

    class AudioBufferChangeListener : public virtual Listener {
    public:
        virtual void bufferChanged() = 0;
    };

    // ------------------------------------------------

    class AnalyzeResultListener : public virtual Listener {
    public:
        virtual void updateAnalyzeResult(const Processing::AnalyzeResult& r) = 0;
    };

    // ------------------------------------------------

    class BufferZoomListener : public virtual Listener {
    public:
        virtual void zoomChanged(Point<float> zoom) = 0;
    };

    // ------------------------------------------------

    class SettingsListener : public virtual Listener {
    public:
        virtual void updateAnalyzeSettings(const Processing::AnalyzeSettings&) {};
        virtual void updateFileLoadSettings(const Processing::FileLoadSettings&) {};
    };

    // ------------------------------------------------

}

// ------------------------------------------------
