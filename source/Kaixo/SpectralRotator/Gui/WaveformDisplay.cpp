
// ------------------------------------------------

#include "Kaixo/SpectralRotator/Gui/WaveformDisplay.hpp"

// ------------------------------------------------

namespace Kaixo::Gui {
    
    // ------------------------------------------------

    constexpr int SincRadius = 16;

    float sinc(float x) {
        if (Math::abs(x) < 1.0e-6f) return 1.f;
        x *= Math::pi;
        return Math::sin(x) / x;
    }

    float blackman(float x) {
        return 0.42f + 0.5f * Math::cos(x) + 0.08f * Math::cos(2.f * x);
    }

    // ------------------------------------------------

    float sampleAtSinc(Processing::SafeAudioBuffer::ReadBuffer& bfr, int center, float r) {
        float sum = 0.f;
        float norm = 0.f;

        for (int i = -SincRadius; i <= SincRadius; ++i) {
            int index = center + i;
            float d = r - static_cast<float>(i);
            float w = sinc(d) * blackman(d * Math::pi / SincRadius);

            float sample = bfr[index].average();

            sum += sample * w;
            norm += w;
        }

        return norm != 0.f ? sum / norm : 0.f;
    }

    float sampleAtLinear(Processing::SafeAudioBuffer::ReadBuffer& bfr, int center, float r) {
        float a = bfr[static_cast<int>(center)].average();
        float b = bfr[static_cast<int>(center) + 1].average();
        return Math::lerp(r, a, b);
    }

    // ------------------------------------------------

    WaveformDisplay::WaveformDisplay(Context c)
        : AudioDisplay(c)
    {}

    // ------------------------------------------------

    AudioFileImage WaveformDisplay::refreshImage(Point<float> visible, Point<int> size) {
        KAIXO_DEBUG("Refreshing image with zoom {} {}.", visible.x(), visible.y());

        const int w = size.x();
        const int h = size.y();

        AudioFileImage result;
        result.image = juce::Image{ juce::Image::PixelFormat::ARGB, w, h, true, juce::SoftwareImageType() };
        result.selection = visible;

        auto& buffer = interface->buffer();

        float sampleRate = buffer.sampleRate();
        float startMillis = visible.x();
        float endMillis = visible.y();

        float startSample = Convert::millisToSamples(startMillis, sampleRate); 
        float endSample = Convert::millisToSamples(endMillis, sampleRate);
        float visibleSamples = endSample - startSample;

        float samplesPerPixel = visibleSamples / Math::max(1.f, w);

        juce::Graphics g(result.image);
        g.setColour(stroke);

        auto sampleToY = [&](float sample) {
            return Math::remap(Math::clamp11(sample), -1.f, 1.f, h, 0.f);
        };

        interface->buffer().access([&](Processing::SafeAudioBuffer::ReadBuffer bfr) {
            // draw min/max envelope
            if (samplesPerPixel > 1.f) {
                for (int x = 0; x < w; ++x) {
                    float s0 = Math::remap(x, 0.f, w, startSample, endSample);
                    float s1 = Math::remap(x + 1, 0.f, w, startSample, endSample);
                    int start = static_cast<int>(Math::floor(s0));
                    int end = static_cast<int>(Math::max(start + 1, Math::ceil(s1))); 
                    
                    float minV = 1.f;
                    float maxV = -1.f;

                    for (int i = start; i < end; ++i) {
                        float v = bfr[i].average();

                        minV = Math::Fast::min(minV, v);
                        maxV = Math::Fast::max(maxV, v);
                    }

                    float startY = sampleToY(minV);
                    float endY = Math::min(sampleToY(maxV), startY - 1);

                    g.drawLine(static_cast<float>(x) + 0.5f, startY, static_cast<float>(x) + 0.5f, endY, 1.f);
                }
            } else { // draw antialiased path
                juce::Path path;

                bool useSinc = samplesPerPixel < 1.f;

                for (int x = 0; x < w; ++x) {
                    double samplePos = Math::remap(x, 0.0, w - 1.0, startSample, endSample);
                    int center = static_cast<int>(samplePos);
                    float r = static_cast<float>(samplePos - center);

                    float value = useSinc ? sampleAtSinc(bfr, center, r) : sampleAtLinear(bfr, center, r);

                    float y = sampleToY(value);

                    if (x == 0) path.startNewSubPath(static_cast<float>(x), y);
                    else path.lineTo(static_cast<float>(x), y);
                }

                g.strokePath(path, juce::PathStrokeType(1.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

                if (samplesPerPixel < 0.125f) {
                    for (int sample = static_cast<int>(startSample); sample < endSample; ++sample) {
                        float a = bfr[sample].average();

                        float x = static_cast<float>(Math::remap(sample, startSample, endSample, double(0.0), w));
                        float y = sampleToY(a);

                        g.fillEllipse({ x - 2, y - 2, 4, 4 });
                    }
                }
            }
        });

        return result;
    }

    // ------------------------------------------------

}

// ------------------------------------------------
