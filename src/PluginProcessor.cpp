#include "PluginProcessor.h"
#include "PluginEditor.h"

inline String spread_value_to_label(float value, int maximumStringLength)
{
    ignoreUnused(maximumStringLength);
    int percentage = roundToInt((value - 0.5f) * 200.0f);
    return String(percentage);
}

inline String float_to_percent_label(float value, int maximumStringLength)
{
    ignoreUnused(maximumStringLength);
    int percentage = roundToInt(value * 100.0f);
    return String(percentage);
}

inline std::unique_ptr<AudioParameterFloat> buildParam(
    const ParameterID &parameterID,
    const String &parameterName,
    float min,
    float max,
    float defaultValue,
    float step,
    const String &parameterLabel,
    std::function<String(float value, int maximumStringLength)> stringFromValue = nullptr)
{
    return std::make_unique<AudioParameterFloat>(
        parameterID,
        parameterName,
        NormalisableRange<float>(min, max, step),
        defaultValue,
        parameterLabel,
        AudioProcessorParameter::genericParameter,
        stringFromValue,
        nullptr);
}

float getParameterValue(AudioProcessorValueTreeState &state, const String &paramID)
{
    auto &param = *state.getParameter(paramID);
    auto range = state.getParameterRange(paramID);
    return range.convertFrom0to1(param.getValue());
}

ChorusAudioProcessor::ChorusAudioProcessor()
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", AudioChannelSet::stereo(), true)
              .withOutput("Output", AudioChannelSet::stereo(), true)),
      state(
          *this, nullptr, "state",
          {std::make_unique<AudioParameterFloat>("rate", "Rate", NormalisableRange<float>(0.0f, 10.0f, 0.01f), 6.5f),
           buildParam("rate_spread", "Rate Spread", 0.01f, 1.0f, 0.95f, 0.01f, "%", float_to_percent_label),
           buildParam("depth", "Depth", 0.0f, 1.0f, 0.25f, 0.01f, "%", float_to_percent_label),
           buildParam("mix", "Mix", 0.0f, 1.0f, 0.5f, 0.01f, "%", float_to_percent_label),
           buildParam("delay", "Delay", 1.0f, 50.0f, 17.0f, 1.0, "ms"),
           buildParam("spread", "Stereo Spread", 0.5f, 1.0f, 0.95f, 0.005f, "%", spread_value_to_label)})
{
    // Add a sub-tree to store the state of our UI
    state.state.addChild({"uiState", {{"width", 400}, {"height", 200}}, {}}, -1, nullptr);
}

ChorusAudioProcessor::~ChorusAudioProcessor()
{
}

const String ChorusAudioProcessor::getName() const
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
    ignoreUnused(index);
}

const String ChorusAudioProcessor::getProgramName(int index)
{
    ignoreUnused(index);
    return {};
}

void ChorusAudioProcessor::changeProgramName(int index, const String &newName)
{
    ignoreUnused(index);
    ignoreUnused(newName);
}

//==============================================================================
void ChorusAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    dsp::ProcessSpec spec = {};
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
    ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono() && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
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

void ChorusAudioProcessor::processBlock(AudioBuffer<float> &buffer, MidiBuffer &midiMessages)
{
    ignoreUnused(midiMessages);
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    dsp::AudioBlock<float> block{buffer};
    auto &chorus = processorChain.get<chorusIndex>();
    chorus.setRate(getParameterValue(state, "rate"));
    chorus.setDepth(getParameterValue(state, "depth"));
    chorus.setMix(getParameterValue(state, "mix"));
    chorus.setDelay(getParameterValue(state, "delay"));
    chorus.setSpread(getParameterValue(state, "spread"));
    chorus.setRateSpread(getParameterValue(state, "rate_spread"));

    processorChain.process(dsp::ProcessContextReplacing<float>(block));
}

//==============================================================================
bool ChorusAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor *ChorusAudioProcessor::createEditor()
{
    return new ChorusAudioProcessorEditor(*this);
}

void ChorusAudioProcessor::getStateInformation(MemoryBlock &destData)
{
    if (auto xmlState = state.copyState().createXml())
    {
        copyXmlToBinary(*xmlState, destData);
    }
}

void ChorusAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    if (auto xmlState = getXmlFromBinary(data, sizeInBytes))
    {
        state.replaceState(ValueTree::fromXml(*xmlState));
    }
}

AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new ChorusAudioProcessor();
}
