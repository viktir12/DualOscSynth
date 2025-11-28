#include "PluginEditor.h"

//==============================================================================
DualOscSynthAudioProcessorEditor::DualOscSynthAudioProcessorEditor (DualOscSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p),
      midiKeyboard (*processor.getMidiKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // Настройка слайдеров
    osc1FreqSlider.setRange(50.0, 2000.0);
    osc1AmpSlider.setRange(0.0, 1.0);
    osc1WaveSlider.setRange(0.0, 4.0);
    osc1WaveSlider.setNumDecimalPlacesToDisplay(0);

    osc2FreqSlider.setRange(50.0, 2000.0);
    osc2AmpSlider.setRange(0.0, 1.0);
    osc2WaveSlider.setRange(0.0, 4.0);
    osc2WaveSlider.setNumDecimalPlacesToDisplay(0);

    cutoffSlider.setRange(20.0, 20000.0);
    cutoffSlider.setSkewFactorFromMidPoint(1000.0f); // ИСПРАВЛЕНО: 1000.0f вместо 1000

    // Attachment'ы для параметров
    osc1FreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "osc1Freq", osc1FreqSlider);
    osc1AmpAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "osc1Amp", osc1AmpSlider);
    osc1WaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "osc1Wave", osc1WaveSlider);
    osc2FreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "osc2Freq", osc2FreqSlider);
    osc2AmpAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "osc2Amp", osc2AmpSlider);
    osc2WaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "osc2Wave", osc2WaveSlider);
    cutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "cutoff", cutoffSlider);

    // Настройка меток
    osc1FreqLabel.setText("Osc1 Freq", juce::dontSendNotification);
    osc1AmpLabel.setText("Osc1 Amp", juce::dontSendNotification);
    osc1WaveLabel.setText("Osc1 Wave", juce::dontSendNotification);
    osc2FreqLabel.setText("Osc2 Freq", juce::dontSendNotification);
    osc2AmpLabel.setText("Osc2 Amp", juce::dontSendNotification);
    osc2WaveLabel.setText("Osc2 Wave", juce::dontSendNotification);
    cutoffLabel.setText("Cutoff", juce::dontSendNotification);

    // Добавление компонентов
    addAndMakeVisible(osc1FreqSlider);
    addAndMakeVisible(osc1AmpSlider);
    addAndMakeVisible(osc1WaveSlider);
    addAndMakeVisible(osc2FreqSlider);
    addAndMakeVisible(osc2AmpSlider);
    addAndMakeVisible(osc2WaveSlider);
    addAndMakeVisible(cutoffSlider);

    addAndMakeVisible(osc1FreqLabel);
    addAndMakeVisible(osc1AmpLabel);
    addAndMakeVisible(osc1WaveLabel);
    addAndMakeVisible(osc2FreqLabel);
    addAndMakeVisible(osc2AmpLabel);
    addAndMakeVisible(osc2WaveLabel);
    addAndMakeVisible(cutoffLabel);

    addAndMakeVisible(midiKeyboard);

    // Запуск таймера для обновления интерфейса
    startTimer(50);

    // Размеры окна
    setSize(750, 400);
}

DualOscSynthAudioProcessorEditor::~DualOscSynthAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void DualOscSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Фон
    g.fillAll (juce::Colours::darkgrey);

    // Заголовок
    g.setColour (juce::Colours::white);
    g.setFont (24.0f);
    g.drawText ("Dual Oscillator Synth", getLocalBounds(), juce::Justification::centredTop, 24);
}

void DualOscSynthAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    auto keyboardHeight = 80;
    auto controlHeight = 60;
    auto labelHeight = 20;
    auto margin = 10;

    // Клавиатура внизу
    midiKeyboard.setBounds(area.removeFromBottom(keyboardHeight));

    // Оставшаяся область для контролов
    area.removeFromBottom(margin);

    // Параметры осциллятора 1
    auto osc1Area = area.removeFromLeft(area.getWidth() / 2);
    osc1FreqLabel.setBounds(osc1Area.removeFromTop(labelHeight));
    osc1FreqSlider.setBounds(osc1Area.removeFromTop(controlHeight));
    osc1AmpLabel.setBounds(osc1Area.removeFromTop(labelHeight));
    osc1AmpSlider.setBounds(osc1Area.removeFromTop(controlHeight));
    osc1WaveLabel.setBounds(osc1Area.removeFromTop(labelHeight));
    osc1WaveSlider.setBounds(osc1Area.removeFromTop(controlHeight));

    // Параметры осциллятора 2 и фильтра
    auto osc2Area = area;
    osc2FreqLabel.setBounds(osc2Area.removeFromTop(labelHeight));
    osc2FreqSlider.setBounds(osc2Area.removeFromTop(controlHeight));
    osc2AmpLabel.setBounds(osc2Area.removeFromTop(labelHeight));
    osc2AmpSlider.setBounds(osc2Area.removeFromTop(controlHeight));
    osc2WaveLabel.setBounds(osc2Area.removeFromTop(labelHeight));
    osc2WaveSlider.setBounds(osc2Area.removeFromTop(controlHeight));
    cutoffLabel.setBounds(osc2Area.removeFromTop(labelHeight));
    cutoffSlider.setBounds(osc2Area.removeFromTop(controlHeight));
}

void DualOscSynthAudioProcessorEditor::timerCallback()
{
    // Обновление интерфейса (если нужно)
    repaint();
}