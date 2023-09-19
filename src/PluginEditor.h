#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "LookAndFeel.h"

class LabeledSlider : public juce::Component,
                      public juce::Slider::Listener
{
public:
    LabeledSlider(const juce::String &name, const juce::String &units)
        : label("", name),
          valueLabel("", "0 " + units),
          units(units)
    {
        addAndMakeVisible(label);
        label.setJustificationType(juce::Justification::centredTop);

        addAndMakeVisible(slider);
        slider.setLookAndFeel(&chickenKnob);
        slider.addListener(this);

        addAndMakeVisible(valueLabel);
        valueLabel.setJustificationType(juce::Justification::centred);
    }

    void resized() override
    {
        float units = 1.0f;
        if (auto *topLevelComponent = getTopLevelComponent())
        {
            int windowWidth = topLevelComponent->getWidth();
            units = ((float)windowWidth / 800.0f);
        }

        label.setFont(Font(25.0f * units, 0));
        valueLabel.setFont(Font(25.0f * units, 0));

        auto bounds = getLocalBounds();
        bounds.reduce(0, 20 * units);
        label.setBounds(bounds.removeFromTop(35 * units));

        valueLabel.setBounds(bounds.removeFromBottom(50 * units));

        auto sliderBounds = bounds.reduced(20 * units).withTrimmedBottom(-20 * units);
        slider.setBounds(sliderBounds);
    }

    void sliderValueChanged(juce::Slider *slider) override
    {
        juce::String valueText = slider->getTextFromValue(slider->getValue());
        valueLabel.setText(valueText + " " + this->units, juce::dontSendNotification);
    }

    juce::Slider slider{
        juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox};

private:
    ChickenKnobStyle chickenKnob;
    juce::Label label;
    juce::Label valueLabel;
    juce::String units;
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

    LabeledSlider rateSlider{"Rate", "hz"};
    LabeledSlider rateSpreadSlider{"Rate Spread", "%"};
    LabeledSlider depthSlider{"Depth", "%"};
    LabeledSlider mixSlider{"Mix", "%"};
    LabeledSlider delaySlider{"Delay", "ms"};
    LabeledSlider spreadSlider{"Stereo Spread", "%"};
    juce::AudioProcessorValueTreeState::SliderAttachment rateAttachment, rateSpreadAttachment, depthAttachment, mixAttachment, delayAttachment, spreadAttachment;

    juce::Value lastUIWidth, lastUIHeight;
    void valueChanged(juce::Value &value) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChorusAudioProcessorEditor)
};
