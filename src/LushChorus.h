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

        dryWet.pushDrySamples(inputBlock);

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

                    SampleType delayedSample = delay[j].popSample((int)channel) * multiplier;
                    SampleType feedbackSample = delayedSample * feedbackAmount;

                    delay[j].pushSample((int)channel, inputSamples[i] + feedbackSample);
                    delay[j].setDelay(delaySamples[j][i]);
                    wet += delayedSample;
                }

                outputSamples[i] = wet / (numberOfDelayLines * 0.5);
            }
        }

        if (enableHighPass)
        {
            auto l = outputBlock.getSingleChannelBlock(0);
            auto r = outputBlock.getSingleChannelBlock(1);
            highPassFilterL.process(juce::dsp::ProcessContextReplacing<SampleType>(l));
            highPassFilterR.process(juce::dsp::ProcessContextReplacing<SampleType>(r));
        }

        dryWet.mixWetSamples(outputBlock);
    }

    void setRate(SampleType rate);
    void setDepth(SampleType depth);
    void setMix(SampleType mix);
    void setDelay(SampleType delay);
    void setSpread(SampleType spread);
    void setRateSpread(SampleType spread);
    void setEnableHighPass(bool enable);
    void setHighPassCutoff(SampleType cutoff);
    void setFeedbackAmount(SampleType feedback);

private:
    void update();
    void updateHighPass();
    double sampleRate = 44100.0;

    static const size_t numberOfDelayLines = 4;

    Lfo<SampleType> lfo[numberOfDelayLines];
    juce::dsp::DelayLine<SampleType, juce::dsp::DelayLineInterpolationTypes::Linear> delay[numberOfDelayLines];
    juce::SmoothedValue<SampleType, juce::ValueSmoothingTypes::Linear> oscVolume;
    juce::AudioBuffer<SampleType> bufferDelayTimes[numberOfDelayLines];
    juce::dsp::IIR::Filter<SampleType> highPassFilterL;
    juce::dsp::IIR::Filter<SampleType> highPassFilterR;
    juce::dsp::DryWetMixer<SampleType> dryWet;

    SampleType rate = 6.5, depth = 0.25, mix = 0.5,
               centreDelay = 17.0, spread = 0.95, rateSpread = 0.95, highPassCutoff = 150.0f, feedbackAmount = 0.0f;

    bool enableHighPass = false;

    static constexpr SampleType maxDepth = 1.0,
                                maxCentreDelayMs = 100.0,
                                oscVolumeMultiplier = 0.2,
                                maximumDelayModulation = 20.0;
};
