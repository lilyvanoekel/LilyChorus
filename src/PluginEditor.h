#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "LookAndFeel.h"

using namespace juce;

class LabeledSlider : public Component,
                      public Slider::Listener
{
public:
    LabeledSlider(const String &name, const String &units)
        : label("", name),
          valueLabel("", "0 " + units),
          units(units)
    {
        addAndMakeVisible(label);
        label.setJustificationType(Justification::centredTop);

        addAndMakeVisible(slider);
        slider.setLookAndFeel(&chickenKnob);
        slider.addListener(this);

        addAndMakeVisible(valueLabel);
        valueLabel.setJustificationType(Justification::centred);
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

    void sliderValueChanged(Slider *slider) override
    {
        String valueText = slider->getTextFromValue(slider->getValue());
        valueLabel.setText(valueText + " " + this->units, dontSendNotification);
    }

    Slider slider{
        Slider::RotaryHorizontalVerticalDrag, Slider::NoTextBox};

private:
    ChickenKnobStyle chickenKnob;
    Label label;
    Label valueLabel;
    String units;
};

class ChorusAudioProcessorEditor : public AudioProcessorEditor, private Value::Listener
{
public:
    ChorusAudioProcessorEditor(ChorusAudioProcessor &);
    ~ChorusAudioProcessorEditor() override;

    void paint(Graphics &) override;
    void resized() override;

private:
    LabeledSlider rateSlider{"Rate", "hz"};
    LabeledSlider rateSpreadSlider{"Rate Spread", "%"};
    LabeledSlider depthSlider{"Depth", "%"};
    LabeledSlider mixSlider{"Mix", "%"};
    LabeledSlider delaySlider{"Delay", "ms"};
    LabeledSlider spreadSlider{"Stereo Spread", "%"};
    AudioProcessorValueTreeState::SliderAttachment rateAttachment, rateSpreadAttachment, depthAttachment, mixAttachment, delayAttachment, spreadAttachment;

    Value lastUIWidth, lastUIHeight;
    void valueChanged(Value &value) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChorusAudioProcessorEditor)
};
