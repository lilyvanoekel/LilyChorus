
#include "LushChorus.h"

template <typename SampleType>
LushChorus<SampleType>::LushChorus()
{
}

template <typename SampleType>
void LushChorus<SampleType>::prepare(const juce::dsp::ProcessSpec &spec)
{
    auto sampleRate = spec.sampleRate;

    const auto maxPossibleDelay = std::ceil((maximumDelayModulation * maxDepth * oscVolumeMultiplier + maxCentreDelayMs) * sampleRate / 1000.0);

    for (size_t i = 0; i < numberOfDelayLines; ++i)
    {
        delay[i] = juce::dsp::DelayLine<SampleType, juce::dsp::DelayLineInterpolationTypes::Linear>{static_cast<int>(maxPossibleDelay)};
        delay[i].prepare(spec);

        bufferDelayTimes[i].setSize(1, (int)spec.maximumBlockSize, false, false, true);

        lfo[i].setSampleRate(sampleRate);
    }

    update();
    reset();
}

template <typename SampleType>
void LushChorus<SampleType>::reset()
{
    for (size_t i = 0; i < numberOfDelayLines; ++i)
    {
        delay[i].reset();
    }

    oscVolume.reset(sampleRate, 0.05);
}

template <typename SampleType>
void LushChorus<SampleType>::update()
{
    for (size_t i = 0; i < numberOfDelayLines; ++i)
    {
        lfo[i].setRate(static_cast<SampleType>(rate / (1.0f + rateSpread * i)));
    }

    oscVolume.setTargetValue(depth * oscVolumeMultiplier);
}

template class LushChorus<float>;
template class LushChorus<double>;
