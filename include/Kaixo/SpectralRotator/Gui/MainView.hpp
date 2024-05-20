
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Interfaces.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------

    class MainView : public View {
    public:

        // ------------------------------------------------

        MainView(Context c);

        // ------------------------------------------------
        
        void resized() override;

        // ------------------------------------------------

        Processing::InterfaceStorage<Processing::FileInterface> inputFileInterface;
        Processing::InterfaceStorage<Processing::FileInterface> rotatedFileInterface;

        // ------------------------------------------------
        
    private:
        constexpr static std::size_t DefaultUI = 1;
        constexpr static std::size_t SettingsOnTheSideUI = 2;

        std::size_t m_UIType = 0;

        std::function<void(void)> m_ResizedCallback;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
