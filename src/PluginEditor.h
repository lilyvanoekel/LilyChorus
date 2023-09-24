#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "LabeledSlider.h"

using namespace juce;

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
