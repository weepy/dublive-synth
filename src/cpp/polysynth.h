#pragma once
#include "synth.h"
#include <vector>
#include <map>

class PolySynth {
public:
    PolySynth(float sampleRate, int maxVoices = 16);
    
    // float process();
    void processBuffer(uintptr_t outputPtr, int bufferSize);
    void noteOn(int midiNote, float velocity);
    void noteOff(int midiNote);
    // void setProperty(const std::string& name, float value);
    void setProperties(const std::map<std::string, float>& props);
    
    void loadWavetable(float key, const std::vector<float>& table);
    
    
private:
    
    std::vector<Synth> voices;
    std::map<std::string, float> properties = {
        {"masterGain", 1.0f},
        {"polyphony", 8.0f}  // Default polyphony setting
    };
    std::map<float, std::vector<float>> wavetables;
    float sampleRate;
    int maxVoices;
    int voiceCounter = 0;  // Add this member variable
    int lastMidiNote = -1;  // Add this to track the last played note

}; 