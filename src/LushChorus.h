#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_core/juce_core.h>

#include "Lfo.h"

// https://www.soundonsound.com/techniques/more-creative-synthesis-delays

template <typename SampleType>
class LushChorus
{
public:
    LushChorus();
    void prepare(const juce::dsp::ProcessSpec &spec);
    void reset();

    template <typename ProcessContext>
    void process(const ProcessContext &context) noexcept
    {
        const auto &inputBlock = context.getInputBlock();
        auto &outputBlock = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples = outputBlock.getNumSamples();
        if (context.isBypassed)
        {
            outputBlock.copyFrom(inputBlock);
            return;
        }

        SampleType *delaySamples[numberOfDelayLines];

        for (size_t i = 0; i < numberOfDelayLines; ++i)
        {
            auto delayValuesBlock = juce::dsp::AudioBlock<SampleType>(bufferDelayTimes[i]).getSubBlock(0, numSamples);
            auto contextDelay = juce::dsp::ProcessContextReplacing<SampleType>(delayValuesBlock);
            delayValuesBlock.clear();

            lfo[i].process(contextDelay);
            delayValuesBlock.multiplyBy(oscVolume);

            delaySamples[i] = bufferDelayTimes[i].getWritePointer(0);

            for (size_t j = 0; j < numSamples; ++j)
            {
                auto lfo = juce::jmax(static_cast<SampleType>(1.0), maximumDelayModulation * delaySamples[i][j] + centreDelay);
                delaySamples[i][j] = static_cast<SampleType>(lfo * sampleRate / 1000.0);
            }
        }

        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            auto *inputSamples = inputBlock.getChannelPointer(channel);
            auto *outputSamples = outputBlock.getChannelPointer(channel);

            for (size_t i = 0; i < numSamples; ++i)
            {
                SampleType wet = 0.0;
                for (size_t j = 0; j < numberOfDelayLines; ++j)
                {
                    size_t favouredChannel = j % numChannels;
                    SampleType multiplier = 1.0 * (1.0 - spread);
                    if (favouredChannel == channel)
                    {
                        multiplier = 1.0 * spread;
                    }
                    delay[j].pushSample((int)channel, inputSamples[i]);
                    delay[j].setDelay(delaySamples[j][i]);
                    wet += delay[j].popSample((int)channel) * multiplier;
                }

                wet = wet / (numberOfDelayLines * 0.5);
                auto output = (inputSamples[i] * (1.0 - mix)) + (wet * mix);

                outputSamples[i] = static_cast<SampleType>(output);
            }
        }
    }

    void setRate(SampleType rate)
    {
        if (rate != this->rate)
        {
            this->rate = rate;
            update();
        }
    }

    void setDepth(SampleType depth)
    {
        if (depth != this->depth)
        {
            this->depth = depth;
            update();
        }
    }

    void setMix(SampleType mix)
    {
        this->mix = mix;
    }

    void setDelay(SampleType delay)
    {
        this->centreDelay = delay;
    }

    void setSpread(SampleType spread)
    {
        this->spread = spread;
    }

    void setRateSpread(SampleType spread)
    {
        this->rateSpread = spread;
        update();
    }

private:
    void update();
    double sampleRate = 44100.0;

    static const size_t numberOfDelayLines = 4;

    Lfo<SampleType> lfo[numberOfDelayLines];
    juce::dsp::DelayLine<SampleType, juce::dsp::DelayLineInterpolationTypes::Linear> delay[numberOfDelayLines];
    juce::SmoothedValue<SampleType, juce::ValueSmoothingTypes::Linear> oscVolume;
    juce::AudioBuffer<SampleType> bufferDelayTimes[numberOfDelayLines];

    SampleType rate = 0.5, depth = 0.25, mix = 0.5,
               centreDelay = 7.0, spread = 0.75, rateSpread = 0.5;

    static constexpr SampleType maxDepth = 1.0,
                                maxCentreDelayMs = 100.0,
                                oscVolumeMultiplier = 0.5,
                                maximumDelayModulation = 20.0;
};
