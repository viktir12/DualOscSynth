cmake_minimum_required(VERSION 3.15)
project(DualOscSynth VERSION 1.0.0)

# Отключаем VST2 ГЛОБАЛЬНО (для JUCE 8.0.5+)
set(JUCE_ENABLE_VST2 OFF CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.5  # СТАБИЛЬНАЯ ВЕРСИЯ
)
FetchContent_MakeAvailable(JUCE)

# ВАЖНО: УБЕРИТЕ VST2 ИЗ ФОРМАТОВ!
juce_add_plugin(DualOscSynth
    PLUGIN_MANUFACTURER_CODE Demo
    PLUGIN_CODE DOSC
    FORMATS VST3 Standalone  # ← ТОЛЬКО VST3 И STANDALONE
    # VST2_ENABLED OFF       # ← ЭТА СТРОКА НЕ НУЖНА
    VST3_CATEGORIES Instrument
    IS_SYNTH TRUE                   # КРИТИЧНО ВАЖНО: это синтезатор
    NEEDS_MIDI_INPUT TRUE           # КРИТИЧНО ВАЖНО: нужен MIDI вход
    PRODUCT_NAME "DualOscSynth"
)

target_sources(DualOscSynth PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/SimpleOscillator.cpp
)
target_include_directories(DualOscSynth PRIVATE Source)

juce_generate_juce_header(DualOscSynth)
target_link_libraries(DualOscSynth PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_extra
)