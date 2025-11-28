#pragma once

#include <JuceHeader.h>
#include <random>
#include <cmath>
#include <algorithm>

class SimpleOscillator
{
public:
    enum class WaveMode { Sine, Saw, Square, Triangle, Noise };

    void setSampleRate(double sr) { 
        sampleRate = sr; 
        calculatePhaseIncrement(); 
    }
    
    void setFrequency(float freq) { 
        frequency = juce::jlimit(20.0f, 20000.0f, freq); 
        calculatePhaseIncrement(); 
    }
    
    void setAmplitude(float amp) { 
        amplitude = juce::jlimit(0.0f, 1.0f, amp); 
    }
    
    void setWaveType(WaveMode mode) { 
        waveMode = mode; 
    }

    float getNextSample()
    {
        float out = 0.0f;

        switch (waveMode)
        {
            case WaveMode::Sine:
                out = std::sin(phase);
                break;
            case WaveMode::Saw:
                out = 2.0f * static_cast<float>(phase / juce::MathConstants<double>::twoPi) - 1.0f;
                break;
            case WaveMode::Square:
                out = (phase < juce::MathConstants<double>::pi) ? 1.0f : -1.0f;
                break;
            case WaveMode::Triangle:
            {
                double t = phase / juce::MathConstants<double>::twoPi;
                out = 2.0f * std::abs(static_cast<float>(2.0 * t - std::floor(2.0 * t + 0.5f))) - 1.0f;
                break;
            }
            case WaveMode::Noise:
                out = noiseDist(noiseGen);
                break;
        }

        // Обновление фазы
        phase += phaseIncrement;
        if (phase >= juce::MathConstants<double>::twoPi)
            phase -= juce::MathConstants<double>::twoPi;

        return out * amplitude;
    }

private:
    void calculatePhaseIncrement()
    {
        phaseIncrement = (frequency * 2.0 * juce::MathConstants<double>::pi) / sampleRate;
    }

    double sampleRate = 44100.0;
    float frequency = 440.0f;
    float amplitude = 0.5f;
    WaveMode waveMode = WaveMode::Sine;
    
    double phase = 0.0;
    double phaseIncrement = 0.0;

    // Для генерации шума
    std::default_random_engine noiseGen{ std::random_device{}() };
    std::uniform_real_distribution<float> noiseDist{ -1.0f, 1.0f };
};