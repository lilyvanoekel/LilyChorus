#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class LabeledSlider : public juce::GroupComponent
{
public:
    LabeledSlider(const juce::String &name)
    {
        setText(name);
        setTextLabelPosition(juce::Justification::centredTop);
        addAndMakeVisible(slider);
    }

    void resized() override
    {
        slider.setBounds(getLocalBounds().reduced(10));
    }

    juce::Slider slider{
        juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow};
};

class ChorusAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Value::Listener
{
public:
    ChorusAudioProcessorEditor(ChorusAudioProcessor &);
    ~ChorusAudioProcessorEditor() override;

        void paint(juce::Graphics &) override;
    void resized() override;

private:
    ChorusAudioProcessor &audioProcessor;

    LabeledSlider rateSlider{"Rate"};
    LabeledSlider rateSpreadSlider{"Rate Spread"};
    LabeledSlider depthSlider{"Depth"};
    LabeledSlider mixSlider{"Mix"};
    LabeledSlider delaySlider{"Delay"};
    LabeledSlider spreadSlider{"Stereo Spread"};
    juce::AudioProcessorValueTreeState::SliderAttachment rateAttachment, rateSpreadAttachment, depthAttachment, mixAttachment, delayAttachment, spreadAttachment;

    juce::Value lastUIWidth, lastUIHeight;
    void valueChanged(juce::Value &value) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChorusAudioProcessorEditor)
};
