
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Interfaces.hpp"
#include "Kaixo/SpectralRotator/Processing/AudioBufferSpectralInformation.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {

    // ------------------------------------------------
    
    class SpectralViewer : public View {
    public:

        // ------------------------------------------------
        
        struct Settings {

            // ------------------------------------------------

            Processing::InterfaceStorage<Processing::FileInterface> file;

            // ------------------------------------------------

        } settings;

        // ------------------------------------------------

        SpectralViewer(Context c, Settings s);
        ~SpectralViewer();

        // ------------------------------------------------
        
        void onIdle() override;

        // ------------------------------------------------
        
        void resized() override;

        // ------------------------------------------------
        
        void paint(juce::Graphics& g) override;

        // ------------------------------------------------
        
        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;

        // ------------------------------------------------
        
        bool keyPressed(const juce::KeyPress& event) override;

        // ------------------------------------------------
        
        void reGenerateImage(bool withAnalyze);

        // ------------------------------------------------
        
    private:
        mutable std::mutex m_AnalyzeResultMutex{};
        juce::Image m_Image = juce::Image(juce::Image::PixelFormat::ARGB, 512, 256, true);
        juce::Image m_Generated = juce::Image(juce::Image::PixelFormat::ARGB, 512, 256, true);

        std::size_t m_AnalyzingProgress = 0;
        std::size_t m_AnalyzingProgressTotal = 0;
        bool m_ShowingProgress = false;
        bool m_DidResize = false;
        std::atomic_bool m_NewImageReady = false;
        std::atomic_bool m_TryingToAssignNewImage = false;
        std::atomic_bool m_GeneratingImage = false;
        Processing::AudioBufferSpectralInformation m_AnalyzeResult;
        float m_PlayPosition = 0;
        Theme::Drawable m_Loading = T.loading;
        std::thread m_GeneratingThread{};

        std::chrono::steady_clock::time_point m_LastResize;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
