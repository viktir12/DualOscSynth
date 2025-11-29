# DualOscSynth
Simple synthesizer for eazy sound-design and sound-programming education.


-------how to build-------
install cmake - https://cmake.org/download/

in powershell:
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release

link to vst3 \build\DualOscSynth_artefacts\Release\VST3
drop dualoscsynth.vst3 in Program Files\Common Files\VST3
in FL Studio -> manage plugins -> find installed plugins, find dualoscsynth, add favorite, open in chanell rack.
Enjoy.
