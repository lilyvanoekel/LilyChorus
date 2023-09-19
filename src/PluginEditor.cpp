#include "PluginProcessor.h"
#include "PluginEditor.h"

ChorusAudioProcessorEditor::ChorusAudioProcessorEditor(ChorusAudioProcessor &p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      rateAttachment(p.state, "rate", rateSlider.slider),
      rateSpreadAttachment(p.state, "rate_spread", rateSpreadSlider.slider),
      depthAttachment(p.state, "depth", depthSlider.slider),
      mixAttachment(p.state, "mix", mixSlider.slider),
      delayAttachment(p.state, "delay", delaySlider.slider),
      spreadAttachment(p.state, "spread", spreadSlider.slider)
{
    addAndMakeVisible(rateSlider);
    addAndMakeVisible(rateSpreadSlider);
    addAndMakeVisible(depthSlider);
    addAndMakeVisible(mixSlider);
    addAndMakeVisible(delaySlider);
    addAndMakeVisible(spreadSlider);

    double ratio = 4.0 / 3.0;
    setResizeLimits(400, 400 / ratio, 2000, 2000 / ratio);
    getConstrainer()->setFixedAspectRatio(ratio);

    setResizable(true, p.wrapperType != juce::AudioProcessor::wrapperType_AudioUnit);

    lastUIWidth.referTo(p.state.state.getChildWithName("uiState").getPropertyAsValue("width", nullptr));
    lastUIHeight.referTo(p.state.state.getChildWithName("uiState").getPropertyAsValue("height", nullptr));

    // set our component's initial size to be the last one that was stored in the filter's settings
    setSize(lastUIWidth.getValue(), lastUIHeight.getValue());

    lastUIWidth.addListener(this);
    lastUIHeight.addListener(this);
}

ChorusAudioProcessorEditor::~ChorusAudioProcessorEditor()
{
}

void ChorusAudioProcessorEditor::paint(juce::Graphics &g)
{
    g.setColour(Colour(0xFF404040));
    g.fillAll();
}

void ChorusAudioProcessorEditor::resized()
{

    juce::FlexBox flexBoxRow1, flexBoxRow2, flexBoxContainer;

    flexBoxRow1.flexDirection = juce::FlexBox::Direction::row;
    flexBoxRow2.flexDirection = juce::FlexBox::Direction::row;
    flexBoxContainer.flexDirection = juce::FlexBox::Direction::column;

    flexBoxRow1.items.addArray({juce::FlexItem(rateSlider).withFlex(1), juce::FlexItem(rateSpreadSlider).withFlex(1), juce::FlexItem(depthSlider).withFlex(1)});
    flexBoxRow2.items.addArray({juce::FlexItem(mixSlider).withFlex(1), juce::FlexItem(delaySlider).withFlex(1), juce::FlexItem(spreadSlider).withFlex(1)});

    flexBoxContainer.items.addArray({juce::FlexItem(flexBoxRow1).withFlex(1), juce::FlexItem(flexBoxRow2).withFlex(1)});

    flexBoxContainer.performLayout(getLocalBounds());

    lastUIWidth = getWidth();
    lastUIHeight = getHeight();
}

void ChorusAudioProcessorEditor::valueChanged(juce::Value &)
{
    setSize(lastUIWidth.getValue(), lastUIHeight.getValue());
}