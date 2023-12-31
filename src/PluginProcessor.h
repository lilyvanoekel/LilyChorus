#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "LushChorus.h"

using namespace juce;

class ChorusAudioProcessor : public AudioProcessor, public AudioProcessorValueTreeState::Listener
#if JucePlugin_Enable_ARA
    ,
                             public AudioProcessorARAExtension
#endif
{
public:
    ChorusAudioProcessor();
    ~ChorusAudioProcessor() override;

    void parameterChanged(const String &parameterID, float newValue) override;
    void updateParams();
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
#endif

    void processBlock(AudioBuffer<float> &, MidiBuffer &) override;

    AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;

    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const String getProgramName(int index) override;
    void changeProgramName(int index, const String &newName) override;

    void getStateInformation(MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;

    AudioProcessorValueTreeState state;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChorusAudioProcessor)

    enum
    {
        chorusIndex
    };
    dsp::ProcessorChain<LushChorus<float>> processorChain;
};
