
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Gui/View.hpp"
#include "Kaixo/Core/Theme/Theme.hpp"

// ------------------------------------------------

#include "Kaixo/SpectralRotator/Processing/Interfaces/FileInterface.hpp"
#include "Kaixo/SpectralRotator/Processing/Utils/AudioBufferSpectralInformation.hpp"

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

        void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

        // ------------------------------------------------
        
        void reGenerateImage(bool withAnalyze, bool inBackground = false);
        void fileWillProbablyChangeSoon() { m_FileWillProbablyChange = true; };
        void fileDidNotChange() { m_FileWillProbablyChange = false; };

        // ------------------------------------------------
        
        void fftSize(std::size_t size);
        void fftResolution(std::size_t range);
        void fftBlockSize(std::size_t ms);
        void fftRange(float range);

        // ------------------------------------------------
        
        Point<float> normalizePosition(Point<float> coord);
        Point<float> denormalizePosition(Point<float> normal);

        Point<float> normalizePositionRelative(Point<float> coord, Rect<float> selection);
        Point<float> denormalizePositionRelative(Point<float> normal, Rect<float> selection);

        // ------------------------------------------------
        
        void select(Rect<float> rect, bool regen = true);

        // ------------------------------------------------

        bool showProgress = true;

        // ------------------------------------------------
        
    private:
        mutable std::mutex m_AnalyzeResultMutex{};
        juce::Image m_Image = juce::Image(juce::Image::PixelFormat::ARGB, 512, 256, true);
        juce::Image m_Generated = juce::Image(juce::Image::PixelFormat::ARGB, 512, 256, true);

        Rect<float> m_Selection{ 0, 10, 10, 48000 };
        Rect<float> m_SelectionWhenStartedGenerating{ 0, 10, 10, 48000 };
        Rect<float> m_ImageSelection{ 0, 10, 10, 48000 };
        Rect<float> m_SelectionWhenStartedDragging{ 0, 10, 10, 48000 };
        std::size_t m_FFTSize = 2048;
        float m_FFTResolution = 1;
        std::size_t m_FFTBlockSize = 50;
        float m_FFTRange = 48;
        std::size_t m_AnalyzingProgress = 0;
        std::size_t m_AnalyzingProgressTotal = 0;
        bool m_ShowingProgress = false;
        bool m_DidResize = false;
        std::atomic_bool m_NewImageReady = false;
        std::atomic_bool m_GeneratingImage = false;
        std::atomic_bool m_AnalyzeInBackground = false;
        std::atomic_bool m_ShouldAnalyze = false;
        std::atomic_bool m_FileWillProbablyChange = false;
        Processing::AudioBufferSpectralInformation m_AnalyzeResult;
        float m_PlayPosition = 0;
        Theme::Drawable m_Loading = T.loading;
        cxxpool::thread_pool m_GeneratingThreadPool{ 1 };

        std::chrono::steady_clock::time_point m_LastResize;

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------
