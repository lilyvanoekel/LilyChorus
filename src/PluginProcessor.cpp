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

inline std::unique_ptr<juce::AudioParameterFloat> buildParam(
    const juce::ParameterID &parameterID,
    const juce::String &parameterName,
    float min,
    float max,
    float defaultValue,
    float step,
    const juce::String &parameterLabel,
    std::function<juce::String(float value, int maximumStringLength)> stringFromValue = nullptr)
{
    return std::make_unique<juce::AudioParameterFloat>(
        parameterID,
        parameterName,
        juce::NormalisableRange<float>(min, max, step),
        defaultValue,
        parameterLabel,
        juce::AudioProcessorParameter::genericParameter,
        stringFromValue,
        nullptr);
}

float getParameterValue(juce::AudioProcessorValueTreeState &state, const juce::String &paramID)
{
    auto &param = *state.getParameter(paramID);
    auto range = state.getParameterRange(paramID);
    return range.convertFrom0to1(param.getValue());
}

ChorusAudioProcessor::ChorusAudioProcessor()
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(
          *this, nullptr, "state",
          {std::make_unique<juce::AudioParameterFloat>("rate", "Rate", juce::NormalisableRange<float>(0.0f, 10.0f), 0.5f),
           buildParam("rate_spread", "Rate Spread", 0.01f, 1.0f, 0.5f, 0.01f, "%", float_to_percent_label),
           buildParam("depth", "Depth", 0.0f, 1.0f, 0.25f, 0.01f, "%", float_to_percent_label),
           buildParam("mix", "Mix", 0.0f, 1.0f, 0.5f, 0.01f, "%", float_to_percent_label),
           buildParam("delay", "Delay", 1.0f, 50.0f, 7.0f, 1.0, "ms"),
           buildParam("spread", "Stereo Spread", 0.5f, 1.0f, 0.9f, 0.005f, "%", spread_value_to_label)})
{
    // Add a sub-tree to store the state of our UI
    state.state.addChild({"uiState", {{"width", 400}, {"height", 200}}, {}}, -1, nullptr);
}

ChorusAudioProcessor::~ChorusAudioProcessor()
{
}

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
    chorus.setRate(getParameterValue(state, "rate"));
    chorus.setDepth(getParameterValue(state, "depth"));
    chorus.setMix(getParameterValue(state, "mix"));
    chorus.setDelay(getParameterValue(state, "delay"));
    chorus.setSpread(getParameterValue(state, "spread"));
    chorus.setRateSpread(getParameterValue(state, "rate_spread"));

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

void ChorusAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    // Store an xml representation of our state.
    if (auto xmlState = state.copyState().createXml())
    {
        copyXmlToBinary(*xmlState, destData);
    }
}

void ChorusAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    // Restore our plug-in's state from the xml representation stored in the above
    // method.
    if (auto xmlState = getXmlFromBinary(data, sizeInBytes))
    {
        state.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new ChorusAudioProcessor();
}
