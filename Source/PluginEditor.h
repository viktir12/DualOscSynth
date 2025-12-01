#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class DualOscSynthAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    DualOscSynthAudioProcessorEditor (DualOscSynthAudioProcessor&);
    ~DualOscSynthAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // Ссылка на процессор
    DualOscSynthAudioProcessor& processor;

    // Элементы интерфейса
    juce::Slider osc1FreqSlider, osc1AmpSlider, osc1WaveSlider;
    juce::Slider osc2FreqSlider, osc2AmpSlider, osc2WaveSlider;
    juce::Slider cutoffSlider;

    juce::Label osc1FreqLabel, osc1AmpLabel, osc1WaveLabel;
    juce::Label osc2FreqLabel, osc2AmpLabel, osc2WaveLabel;
    juce::Label cutoffLabel;

    juce::MidiKeyboardComponent midiKeyboard;

    // Attachment'ы для параметров
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc1FreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc1AmpAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc1WaveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc2FreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc2AmpAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc2WaveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DualOscSynthAudioProcessorEditor)
};