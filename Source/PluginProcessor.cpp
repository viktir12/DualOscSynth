#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DualOscSynthAudioProcessor::DualOscSynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

DualOscSynthAudioProcessor::~DualOscSynthAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout DualOscSynthAudioProcessor::createParams()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Осциллятор 1
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc1Freq", "Osc1 Freq", juce::NormalisableRange<float> (50.0f, 2000.0f, 1.0f, 0.3f), 440.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc1Amp", "Osc1 Amp", 0.0f, 1.0f, 0.3f));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("osc1Wave", "Osc1 Wave",
        juce::StringArray { "Sine", "Saw", "Square", "Triangle", "Noise" }, 0));

    // Осциллятор 2
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc2Freq", "Osc2 Freq", juce::NormalisableRange<float> (50.0f, 2000.0f, 1.0f, 0.3f), 550.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc2Amp", "Osc2 Amp", 0.0f, 1.0f, 0.2f));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("osc2Wave", "Osc2 Wave",
        juce::StringArray { "Sine", "Saw", "Square", "Triangle", "Noise" }, 0));

    // Фильтр
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("cutoff", "Cutoff", juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.3f), 5000.0f));

    return { params.begin(), params.end() };
}

//==============================================================================
void DualOscSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    // Инициализация осцилляторов
    osc1.setSampleRate(sampleRate);
    osc2.setSampleRate(sampleRate);

    // Инициализация сглаживания
    const double smoothingTimeSeconds = 0.02;
    ampSmoothed1.reset(sampleRate, smoothingTimeSeconds);
    ampSmoothed2.reset(sampleRate, smoothingTimeSeconds);
    cutoffSmoothed.reset(sampleRate, smoothingTimeSeconds);

    // Сброс состояния фильтра
    filterStateLeft = 0.0f;
    filterStateRight = 0.0f;
    
    // Инициализация целевых значений
    updateSmoothedTargets();
}

void DualOscSynthAudioProcessor::updateSmoothedTargets()
{
    ampSmoothed1.setTargetValue(*apvts.getRawParameterValue("osc1Amp"));
    ampSmoothed2.setTargetValue(*apvts.getRawParameterValue("osc2Amp"));
    cutoffSmoothed.setTargetValue(*apvts.getRawParameterValue("cutoff"));
}

void DualOscSynthAudioProcessor::releaseResources()
{
    // Очистка ресурсов (если нужно)
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DualOscSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
   #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
   #else
    // Поддерживаем только стерео выход
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // Не поддерживаем входы
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::disabled())
        return false;

    return true;
   #endif
}
#endif

//==============================================================================
void DualOscSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Очистка входных каналов
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Отладочный вывод для проверки получения MIDI (только в debug сборке)
    #if JUCE_DEBUG
    if (midiMessages.getNumEvents() > 0)
    {
        DBG("MIDI events: " + String(midiMessages.getNumEvents()));
    }
    #endif
    
    // Обработка MIDI через MidiKeyboardState
    midiKeyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);

    // Обновляем целевые значения для плавного изменения параметров
    updateSmoothedTargets();
    
    // Определение активной ноты (монофонический режим)
    int newMidiNote = -1;
    bool anyNoteOn = false;
    
    // Проверяем все ноты от высоких к низким
    for (int note = 127; note >= 0; --note)
    {
        // Проверяем все каналы
        for (int channel = 1; channel <= 16; ++channel)
        {
            if (midiKeyboardState.isNoteOn(channel, note))
            {
                anyNoteOn = true;
                newMidiNote = note;
                break; // нашли самую высокую ноту
            }
        }
        if (anyNoteOn) break;
    }

    bool noteChanged = (newMidiNote != currentMidiNote);
    bool wasOn = (currentMidiNote >= 0);

    if (anyNoteOn && noteChanged)
    {
        // Обновление частоты при новой ноте
        float baseFreq = juce::MidiMessage::getMidiNoteInHertz(static_cast<float>(newMidiNote));
        osc1.setFrequency(baseFreq);
        osc2.setFrequency(baseFreq * 1.5f); // фиксированный интервал (квинта)
        currentMidiNote = newMidiNote;
        anyNoteCurrentlyPlaying = true;
    }
    else if (!anyNoteOn && wasOn)
    {
        // Сброс при отпускании всех нот
        currentMidiNote = -1;
        anyNoteCurrentlyPlaying = false;
        // Сброс состояния фильтра для предотвращения щелчков
        filterStateLeft = 0.0f;
        filterStateRight = 0.0f;
    }

    // Получаем указатели на каналы
    float* leftChannel  = buffer.getWritePointer(0);
    float* rightChannel = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : leftChannel;

    int numSamples = buffer.getNumSamples();

    // Кэшируем параметры формы волны (обновляем только при изменении)
    static int lastOsc1Wave = -1;
    static int lastOsc2Wave = -1;
    int currentOsc1Wave = static_cast<int>(*apvts.getRawParameterValue("osc1Wave"));
    int currentOsc2Wave = static_cast<int>(*apvts.getRawParameterValue("osc2Wave"));
    
    if (currentOsc1Wave != lastOsc1Wave)
    {
        osc1.setWaveType(static_cast<SimpleOscillator::WaveMode>(currentOsc1Wave));
        lastOsc1Wave = currentOsc1Wave;
    }
    
    if (currentOsc2Wave != lastOsc2Wave)
    {
        osc2.setWaveType(static_cast<SimpleOscillator::WaveMode>(currentOsc2Wave));
        lastOsc2Wave = currentOsc2Wave;
    }

    // Цикл по семплам
    for (int i = 0; i < numSamples; ++i)
    {
        // Обновляем параметры с плавной интерполяцией
        float amp1 = ampSmoothed1.getNextValue();
        float amp2 = ampSmoothed2.getNextValue();
        float cutoff = cutoffSmoothed.getNextValue();

        // Если нет нажатых клавиш - выключаем звук
        if (!anyNoteCurrentlyPlaying)
        {
            amp1 = 0.0f;
            amp2 = 0.0f;
        }

        // Ограничение минимальной частоты среза для стабильности фильтра
        cutoff = juce::jmax(20.0f, cutoff);
        cutoff = juce::jmin(static_cast<float>(currentSampleRate) * 0.45f, cutoff); // Предотвращение нестабильности

        // Расчёт коэффициента фильтра (однополюсный LPF)
        const float twoPiFc = 2.0f * juce::MathConstants<float>::pi * cutoff;
        const float dt = 1.0f / static_cast<float>(currentSampleRate);
        const float rc = 1.0f / twoPiFc;
        const float alpha = dt / (rc + dt);
        
        // Обновление амплитуды осцилляторов
        osc1.setAmplitude(amp1);
        osc2.setAmplitude(amp2);

        // Генерация сигнала (только если есть нажатые клавиши)
        float osc1Out = anyNoteCurrentlyPlaying ? osc1.getNextSample() : 0.0f;
        float osc2Out = anyNoteCurrentlyPlaying ? osc2.getNextSample() : 0.0f;

        // Смешивание
        float mix = (osc1Out + osc2Out) * 0.5f;

        // Применение фильтра
        filterStateLeft  = alpha * mix + (1.0f - alpha) * filterStateLeft;
        filterStateRight = alpha * mix + (1.0f - alpha) * filterStateRight;

        // Запись в буфер
        leftChannel[i]  = filterStateLeft;
        rightChannel[i] = filterStateRight;
    }
}

//==============================================================================
bool DualOscSynthAudioProcessor::hasEditor() const
{
    return true; // Всегда возвращаем true
}

juce::AudioProcessorEditor* DualOscSynthAudioProcessor::createEditor()
{
    return new DualOscSynthAudioProcessorEditor (*this);
}

//==============================================================================
const juce::String DualOscSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DualOscSynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DualOscSynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DualOscSynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DualOscSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

//==============================================================================
int DualOscSynthAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DualOscSynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DualOscSynthAudioProcessor::setCurrentProgram (int index)
{
    ignoreUnused (index);
}

const juce::String DualOscSynthAudioProcessor::getProgramName (int index)
{
    ignoreUnused (index);
    return {};
}

void DualOscSynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    ignoreUnused (index, newName);
}

//==============================================================================
void DualOscSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Сохраняем состояние APVTS
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void DualOscSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Загружаем состояние APVTS
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DualOscSynthAudioProcessor();
}