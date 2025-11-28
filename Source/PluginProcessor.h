#pragma once

#include <JuceHeader.h>
#include "SimpleOscillator.h"

//==============================================================================
class DualOscSynthAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    DualOscSynthAudioProcessor();
    ~DualOscSynthAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    static juce::AudioProcessorValueTreeState::ParameterLayout createParams();

    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "PARAMS", createParams()};

    // ДОБАВЛЕНО: публичный доступ к MIDI-состоянию для редактора
    juce::MidiKeyboardState* getMidiKeyboardState() {
        return &midiKeyboardState;
    }

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DualOscSynthAudioProcessor)

    // Осцилляторы
    SimpleOscillator osc1;
    SimpleOscillator osc2;

    // Состояние MIDI
    juce::MidiKeyboardState midiKeyboardState;
    int currentMidiNote = -1;

    // Состояние фильтра
    float filterStateLeft = 0.0f;
    float filterStateRight = 0.0f;

    // Сглаженные параметры
    juce::SmoothedValue<float> ampSmoothed1 {0.0f};
    juce::SmoothedValue<float> ampSmoothed2 {0.0f};
    juce::SmoothedValue<float> cutoffSmoothed {5000.0f};

    double currentSampleRate = 44100.0;
};