#pragma once

#include <math.h>

template <typename SampleType>
class Lfo
{
    SampleType lfoX;
    SampleType lfoY;
    SampleType lfoE;
    SampleType pi;
    SampleType twoPi;
    SampleType lfoRate;
    SampleType sampleRate;
    int normalizationCounter;

    void updateLfo()
    {
        SampleType omega = twoPi * lfoRate / sampleRate;
        lfoE = 2 * sin(omega / 2.0);
    }

    void normalize()
    {
        SampleType magnitude = sqrt(lfoX * lfoX + lfoY * lfoY);
        if (magnitude != 0.0)
        {
            lfoX /= magnitude;
            lfoY /= magnitude;
        }
    }

public:
    Lfo() : pi(2 * acos(static_cast<SampleType>(0.0))),
            twoPi(2 * pi),
            lfoRate(static_cast<SampleType>(1.0)),
            sampleRate(static_cast<SampleType>(44100.0)),
            lfoE(static_cast<SampleType>(0.0)),
            normalizationCounter(0)
    {
        lfoX = cos(static_cast<SampleType>(0.0));
        lfoY = sin(static_cast<SampleType>(0.0));
    }

    void setSampleRate(SampleType rate)
    {
        sampleRate = rate;
        updateLfo();
    }

    void setRate(SampleType rate)
    {
        lfoRate = rate;
        updateLfo();
    }

    template <typename ProcessContext>
    void process(const ProcessContext &context) noexcept
    {
        auto &outputBlock = context.getOutputBlock();
        const auto numSamples = outputBlock.getNumSamples();

        auto *outputSamples = outputBlock.getChannelPointer(0);
        normalize();

        for (size_t i = 0; i < numSamples; ++i)
        {
            lfoX = lfoX - lfoE * lfoY;
            lfoY = lfoE * lfoX + lfoY;
            outputSamples[i] = lfoX;
        }
    }
};
