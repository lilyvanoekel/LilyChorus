/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

inline juce::String spread_value_to_label(float value, int maximumStringLength)
{
    int percentage = juce::roundToInt((value - 0.5f) * 200.0f);
    return juce::String(percentage);
}

inline juce::String float_to_percent_label(float value, int maximumStringLength)
{
    int percentage = juce::roundToInt(value * 100.0f);
    return juce::String(percentage);
}

inline juce::AudioParameterFloat *buildParam(
    const juce::ParameterID &parameterID,
    const juce::String &parameterName,
    float min,
    float max,
    float defaultValue,
    float step,
    const juce::String &parameterLabel,
    std::function<juce::String(float value, int maximumStringLength)> stringFromValue = nullptr)
{
    return new juce::AudioParameterFloat(
        parameterID,
        parameterName,
        juce::NormalisableRange<float>(min, max, step),
        defaultValue,
        parameterLabel,
        juce::AudioProcessorParameter::genericParameter,
        stringFromValue,
        nullptr);
}

//==============================================================================
ChorusAudioProcessor::ChorusAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      )
#endif
{
    addParameter(rate = new juce::AudioParameterFloat("rate", "Rate", 0.0f, 10.0f, 0.5f));
    addParameter(rateSpread = buildParam("ratespread", "Rate Spread", 0.01f, 1.0f, 0.5f, 0.01f, "%", float_to_percent_label));
    addParameter(depth = buildParam("depth", "Depth", 0.0f, 1.0f, 0.25f, 0.01f, "%", float_to_percent_label));
    addParameter(mix = buildParam("mix", "Mix", 0.0f, 1.0f, 0.5f, 0.01f, "%", float_to_percent_label));
    addParameter(delay = buildParam("delay", "Delay", 1.0f, 50.0f, 7.0f, 1.0, "ms"));
    addParameter(spread = buildParam("spread", "Stereo Spread", 0.5f, 1.0f, 0.9f, 0.005f, "%", spread_value_to_label));
}

ChorusAudioProcessor::~ChorusAudioProcessor()
{
}

//==============================================================================
const juce::String ChorusAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ChorusAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool ChorusAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool ChorusAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double ChorusAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ChorusAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
              // so this should be at least 1, even if you're not really implementing programs.
}

int ChorusAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ChorusAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String ChorusAudioProcessor::getProgramName(int index)
{
    return {};
}

void ChorusAudioProcessor::changeProgramName(int index, const juce::String &newName)
{
}

//==============================================================================
void ChorusAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec = {};
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();

    processorChain.prepare(spec);
}

void ChorusAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ChorusAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

        // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void ChorusAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    juce::dsp::AudioBlock<float> block{buffer};
    auto &chorus = processorChain.get<chorusIndex>();
    chorus.setRate(*rate);
    chorus.setDepth(*depth);
    chorus.setMix(*mix);
    chorus.setDelay(*delay);
    chorus.setSpread(*spread);
    chorus.setRateSpread(*rateSpread);

    processorChain.process(juce::dsp::ProcessContextReplacing<float>(block));
}

//==============================================================================
bool ChorusAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *ChorusAudioProcessor::createEditor()
{
    return new ChorusAudioProcessorEditor(*this);
}

//==============================================================================
void ChorusAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    std::unique_ptr<juce::XmlElement> xml(new juce::XmlElement("ParamTutorial"));
    xml->setAttribute("rate", (double)*rate);
    xml->setAttribute("depth", (double)*depth);
    xml->setAttribute("mix", (double)*mix);
    xml->setAttribute("delay", (double)*delay);
    xml->setAttribute("spread", (double)*spread);
    xml->setAttribute("rate_spread", (double)*rateSpread);
    copyXmlToBinary(*xml, destData);
}

void ChorusAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName("ParamTutorial"))
        {
            *rate = (float)xmlState->getDoubleAttribute("rate", 0.5);
            *depth = (float)xmlState->getDoubleAttribute("depth", 0.25);
            *mix = (float)xmlState->getDoubleAttribute("mix", 0.5);
            *delay = (float)xmlState->getDoubleAttribute("delay", 7.0);
            *spread = (float)xmlState->getDoubleAttribute("spread", 0.75);
            *rateSpread = (float)xmlState->getDoubleAttribute("rate_spread", 0.5);
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new ChorusAudioProcessor();
}
