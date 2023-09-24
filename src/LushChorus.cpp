
#include "LushChorus.h"

template <typename SampleType>
LushChorus<SampleType>::LushChorus()
{
    dryWet.setMixingRule(juce::dsp::DryWetMixingRule::linear);
}

template <typename SampleType>
void LushChorus<SampleType>::prepare(const juce::dsp::ProcessSpec &spec)
{
    sampleRate = spec.sampleRate;

    const auto maxPossibleDelay = std::ceil((maximumDelayModulation * maxDepth * oscVolumeMultiplier + maxCentreDelayMs) * sampleRate / 1000.0);
    dryWet.prepare(spec);

    for (size_t i = 0; i < numberOfDelayLines; ++i)
    {
        delay[i] = juce::dsp::DelayLine<SampleType, juce::dsp::DelayLineInterpolationTypes::Linear>{static_cast<int>(maxPossibleDelay)};
        delay[i].prepare(spec);

        bufferDelayTimes[i].setSize(1, (int)spec.maximumBlockSize, false, false, true);

        lfo[i].setSampleRate(sampleRate);
    }

    update();
    reset();
    updateHighPass();
}

template <typename SampleType>
void LushChorus<SampleType>::reset()
{
    for (size_t i = 0; i < numberOfDelayLines; ++i)
    {
        delay[i].reset();
    }

    oscVolume.reset(sampleRate, 0.05);
    highPassFilterL.reset();
    highPassFilterR.reset();
    dryWet.reset();
}

template <typename SampleType>
void LushChorus<SampleType>::update()
{
    for (size_t i = 0; i < numberOfDelayLines; ++i)
    {
        lfo[i].setRate(static_cast<SampleType>(rate / (1.0f + rateSpread * i)));
    }

    oscVolume.setTargetValue(depth * oscVolumeMultiplier);
    dryWet.setWetMixProportion(mix);
}

template <typename SampleType>
void LushChorus<SampleType>::updateHighPass()
{
    double qFactor = 0.7071;
    highPassFilterL.coefficients = juce::dsp::IIR::Coefficients<SampleType>::makeHighPass(sampleRate, highPassCutoff, qFactor);
    highPassFilterR.coefficients = juce::dsp::IIR::Coefficients<SampleType>::makeHighPass(sampleRate, highPassCutoff, qFactor);
}

template <typename SampleType>
void LushChorus<SampleType>::setRate(SampleType rate)
{
    if (rate != this->rate)
    {
        this->rate = rate;
        update();
    }
}

template <typename SampleType>
void LushChorus<SampleType>::setDepth(SampleType depth)
{
    if (depth != this->depth)
    {
        this->depth = depth;
        update();
    }
}

template <typename SampleType>
void LushChorus<SampleType>::setMix(SampleType mix)
{
    if (mix != this->mix)
    {
        this->mix = mix;
        update();
    }
}

template <typename SampleType>
void LushChorus<SampleType>::setDelay(SampleType delay)
{
    this->centreDelay = delay;
}

template <typename SampleType>
void LushChorus<SampleType>::setSpread(SampleType spread)
{
    this->spread = spread;
}

template <typename SampleType>
void LushChorus<SampleType>::setRateSpread(SampleType spread)
{
    if (spread != this->rateSpread)
    {
        this->rateSpread = spread;
        update();
    }
}

template <typename SampleType>
void LushChorus<SampleType>::setEnableHighPass(bool enable)
{
    enableHighPass = enable;
}

template <typename SampleType>
void LushChorus<SampleType>::setHighPassCutoff(SampleType cutoff)
{
    if (cutoff != highPassCutoff)
    {
        highPassCutoff = cutoff;
        updateHighPass();
    }
}

template <typename SampleType>
void LushChorus<SampleType>::setEnableDrive(bool enable)
{
    enableDrive = enable;
}

template class LushChorus<float>;
template class LushChorus<double>;
