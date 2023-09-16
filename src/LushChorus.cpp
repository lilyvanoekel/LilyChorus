
#include "LushChorus.h"

template <typename SampleType>
LushChorus<SampleType>::LushChorus()
{
    auto oscFunction = [](SampleType x) { return std::sin(x); };
    for (size_t i = 0; i < numberOfDelayLines; ++i) {
        osc[i].initialise(oscFunction);
    }
}

template <typename SampleType>
void LushChorus<SampleType>::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    const auto maxPossibleDelay = std::ceil((maximumDelayModulation * maxDepth * oscVolumeMultiplier + maxCentreDelayMs)
        * sampleRate / 1000.0);
    
    for (size_t i = 0; i < numberOfDelayLines; ++i) {
        delay[i] = juce::dsp::DelayLine<SampleType, juce::dsp::DelayLineInterpolationTypes::Linear>{ static_cast<int> (maxPossibleDelay) };
        delay[i].prepare(spec);

        bufferDelayTimes[i].setSize(1, (int)spec.maximumBlockSize, false, false, true);

        osc[i].prepare(spec);
    }

    update();
    reset();
}

template <typename SampleType>
void LushChorus<SampleType>::reset()
{
    for (size_t i = 0; i < numberOfDelayLines; ++i) {
        delay[i].reset();
        osc[i].reset();
    }
    
    oscVolume.reset(sampleRate, 0.05);


}

template <typename SampleType>
void LushChorus<SampleType>::update()
{
    for (size_t i = 0; i < numberOfDelayLines; ++i) {
        osc[i].setFrequency(static_cast<SampleType>(rate / (i + 1)));
    }
    
    oscVolume.setTargetValue(depth * oscVolumeMultiplier);
}

template class LushChorus<float>;
template class LushChorus<double>;