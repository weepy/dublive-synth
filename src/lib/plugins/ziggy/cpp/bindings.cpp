#include <emscripten/bind.h>
#include <emscripten/val.h>
#include "polysynth.h"

using namespace emscripten;

// EMSCRIPTEN_BINDINGS(synth_module) {
//     enum_<Synth::Waveform>("Waveform")
//         .value("Sine", Synth::Waveform::Sine)
//         .value("Square", Synth::Waveform::Square)
//         .value("Saw", Synth::Waveform::Saw);
        
//     class_<Synth>("Synth")
//         .constructor<float>()
//         .function("process", &Synth::process)
//         .function("noteOn", &Synth::noteOn)
//         .function("noteOff", &Synth::noteOff)
//         .function("setWaveform", &Synth::setWaveform)
//         .function("setFilterCutoff", &Synth::setFilterCutoff)
//         .function("setFilterResonance", &Synth::setFilterResonance);
// }

// Helper function to convert JS Float32Array to std::vector<float>
void loadWavetableHelper(PolySynth& synth, float key, const val& array) {
    size_t length = array["length"].as<size_t>();
    std::vector<float> wavetable(length);
    
    // Directly copy values from the array
    for (size_t i = 0; i < length; i++) {
        wavetable[i] = array[i].as<float>();
    }
    
    synth.loadWavetable(key, wavetable);
}

// Helper function to convert JS object to std::map
void setPropertiesHelper(PolySynth& synth, const val& obj) {
    std::map<std::string, float> props;
    
    // Get all enumerable properties from the object
    val keys = val::global("Object").call<val>("keys", obj);
    int length = keys["length"].as<int>();
    
    for (int i = 0; i < length; i++) {
        std::string key = keys[i].as<std::string>();
        float value = obj[key].as<float>();
        props[key] = value;
    }
    
    synth.setProperties(props);
}

EMSCRIPTEN_BINDINGS(polysynth_module) {
    // enum_<Synth::Waveform>("Waveform")
    //     .value("Sine", Synth::Waveform::Sine)
    //     .value("Square", Synth::Waveform::Square)
    //     .value("Saw", Synth::Waveform::Saw);

    class_<PolySynth>("PolySynth")
        .constructor<float, int>()
        // .function("process", &PolySynth::process)
        .function("processBuffer", &PolySynth::processBuffer)
        .function("noteOn", &PolySynth::noteOn)
        .function("noteOff", &PolySynth::noteOff)
        .function("loadWavetable", &loadWavetableHelper)
        // .function("setProperty", &PolySynth::setProperty)
        .function("setProperties", &setPropertiesHelper);
        // .function("getProperty", &PolySynth::getProperty);
} 