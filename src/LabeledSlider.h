#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "LookAndFeel.h"

using namespace juce;

class LabeledSlider : public Component,
                      public Slider::Listener,
                      public Timer
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
        doLayout();
        startTimer(10);
    }

    void sliderValueChanged(Slider *slider) override
    {
        String valueText = slider->getTextFromValue(slider->getValue());
        valueLabel.setText(valueText + " " + this->units, dontSendNotification);
    }

    Slider slider{
        Slider::RotaryHorizontalVerticalDrag, Slider::NoTextBox};

    void timerCallback() override
    {
        doLayout();
        stopTimer();
    }

    ~LabeledSlider()
    {
        stopTimer();
    }

private:
    ChickenKnobStyle chickenKnob;
    Label label;
    Label valueLabel;
    String units;

    void doLayout()
    {
        float units = 1.0f;
        if (auto *topLevelComponent = getTopLevelComponent())
        {
            int windowWidth = topLevelComponent->getWidth();
            units = ((float)windowWidth / 800.0f);
        }

        label.setFont(Font(14.0f * units, 0));
        valueLabel.setFont(Font(14.0f * units, 0));

        auto bounds = getLocalBounds();
        bounds.reduce(0, 12 * units);
        label.setBounds(bounds.removeFromTop(20 * units));

        valueLabel.setBounds(bounds.removeFromBottom(20 * units));

        auto sliderBounds = bounds.reduced(10 * units).withTrimmedBottom(-10 * units);
        slider.setBounds(sliderBounds);
    }
};