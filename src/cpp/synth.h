#ifndef SYNTH_H
#define SYNTH_H

#include <vector>
#include <map>
#include <string>
#include "filter_coefficients.h"

class ADSR {
public:
    ADSR() = default;
    
    void setParameters(float attack, float decay, float sustain, float release) {
        this->attack = attack;
        this->decay = decay;
        this->sustain = sustain;
        this->release = release;
    }
    
    void noteOn() {
        isNoteOn = true;
        position = 0.0f;
        level = 0.0f;
    }
    
    void noteOff() {
        isNoteOn = false;
        noteOffTime = position;
        releaseStartLevel = level;
    }
    
    void abort() {
        isNoteOn = false;
        noteOffTime = position;
        releaseStartLevel = level;
        // Store original release time and set to quick release
        originalRelease = release;
        release = 0.05f; // 50ms abort time
    }
    
    float process(float deltaTime) {
        position += deltaTime;
        level = calculateLevel(position);
        
        // Restore original release time once envelope is complete
        if (!isActive() && release != originalRelease) {
            release = originalRelease;
        }
        return level;
    }
    
    bool isActive() const {
        return isNoteOn || level > 0.0001f;
    }
    
private:
    float calculateLevel(float time) {
        float level = 0.0f;
        
        if (isNoteOn) {
            if (time < attack) {
                level = time / attack + 0.00001f;
            } else if (time < attack + decay) {
                float decayPhase = time - attack;
                level = 1.0f - (1.0f - sustain) * (decayPhase / decay) + 0.00001f;
            } else {
                level = sustain;
            }
        } else {
            float releasePhase = time - noteOffTime;
            if (releasePhase < release) {
                level = releaseStartLevel * (1.0f - releasePhase / release);
            } else {
                level = 0.0f;
            }
        }
        
        return level;
    }
    
    float attack = 0.1f;
    float decay = 0.1f;
    float sustain = 0.7f;
    float release = 0.1f;
    float position = 0.0f;
    float level = 0.0f;
    float noteOffTime = 0.0f;
    float releaseStartLevel = 0.0f;
    bool isNoteOn = false;
    float originalRelease = 0.1f;  // Add this new member variable
};

// Add this enum before the Synth class
enum class LFOWaveform {
    Triangle,
    Sawtooth,
    Square,
    SampleAndHold,
    Sine
};

// Update the enum to include destination options
enum class LFODestination {
    Oscillators,
    Filter,
    FM,
    Phase,
    Mix
};

class Synth {
public:
    enum class Waveform {
        Sine,
        Square,
        Saw
    };

    Synth(float sampleRate = 44100.0f, std::map<float, std::vector<float>>* wavetables = nullptr);
    
    // float process();
    void processBuffer(float* buffer, int bufferSize);
    void noteOn(int midiNote, float velocity, int fromMidiNote = -1);
    void noteOff();
    void setProperties(const std::map<std::string, float>& props);
    // void setWavetable(const std::vector<float>& table);
    
    float calculateEnvelopeLevel(float time);
    bool isVoiceActive();
    bool isActive = false;
    bool isAborting = false;  // Add this new member
    int midiNote = -1;
    bool noteIsOn = false;
    float noteOffTime = 0.0f;
    float wavetableKey = 0.0f;
    float releaseStartLevel = 0.0f;
    int startTime = 0;
    std::map<float, std::vector<float>>* wavetables = nullptr;  // Pointer to wavetables map
    void abort();  // Add this declaration

private:
    // Add these new member variables for noise generator
    float noiseValue = 0.0f;
    float noisePhase = 0.0f;
    float noiseLowpassOutput = 0.0f;
    
    // Helper method for noise generation
    float processNoise(float deltaTime, float color);
    
    std::map<std::string, float> properties = {
        // Amplitude envelope parameters
        {"ampAttack", 0.5f},
        {"ampDecay", 0.1f},
        {"ampSustain", 0.7f},
        {"ampRelease", 0.9f},
        
        // Filter envelope parameters
        {"filterAttack", 0.1f},
        {"filterDecay", 0.1f},
        {"filterSustain", 0.7f},
        {"filterRelease", 0.1f},
        {"filterEnvAmount", 0.5f},
        {"lastFilterType", 0.0f},
        
        // Filter parameters
        {"cutoff", 1000.0f},
        {"resonance", 0.0f},
        
        // Oscillator parameters
        {"wave1", 0.0f},
        {"phaseOffset1", 0.0f},
        {"phaseMode1", 0.0f},
        {"wave2", 0.0f},
        {"phaseOffset2", 0.0f},
        {"phaseMode2", 0.0f},
        {"oct1", 0.0f},
        {"oct2", 0.0f},
        {"semi1", 0.0f},
        {"semi2", 0.0f},
        {"cent1", 0.0f},
        {"cent2", 0.0f},
        {"fmAmount", 0.0f},
        {"mix", 0.5f},
        
        // Simplified LFO parameters
        {"lfoSync", 0.0f},      // 0 = free, 1 = sync
        {"lfoRate", 0.5f},      // 0-1 range
        {"lfoAmount", 0.0f},    // 0-1 range
        {"lfoWaveform", 0.0f},  // 0=Triangle, 1=Saw, 2=Square, 3=S&H, 4=Sine
        {"lfoDestination", 0.0f}, // 0=Osc, 1=Filter, 2=FM, 3=Phase, 4=Mix
        
        // Add noise generator parameters
        {"noiseDecay", 0.1f},
        {"noiseColor", 1.0f},  // 0 = dark, 1 = bright
        {"noiseLevel", 0.0f},
        {"noiseEnabled", 0.0f},
    };

    float sampleRate;
    float phase1 = 0.0f;  // Phase for oscillator 1
    float phase2 = 0.0f;  // Phase for oscillator 2
    float envLevel = 0.f;
    float frequency = 440.0f;
    float velocity = 0.0f;
    float envPos = 0.0f;
    float filterState = 0.0f;
    
    // std::vector<float> wavetable;
    // size_t wavetableSize = 0;
    
    const std::vector<float>* wave1 = nullptr;
    const std::vector<float>* wave2 = nullptr;
    
    float processOscillator(const std::vector<float>* wave, float phaseOffset, int phaseMode, float freq, float& phase);
    float processEnvelope();
    void processFilter(float* input, int numSamples, float cutoff01);
    void updateWavetable();

    // Filter state variables
    float x1 = 0.0f, x2 = 0.0f;  // Input history
    float y1 = 0.0f, y2 = 0.0f;  // Output history
    
    // Cached filter coefficients
    float b0 = 0.0f, b1 = 0.0f, b2 = 0.0f;
    float a1 = 0.0f, a2 = 0.0f;
    float lastCutoff = -1.0f;
    float lastResonance = -1.0f;

    ADSR ampEnv;
    ADSR filterEnv;

    // Add baseCutoff member variable
    // float baseCutoff = 1000.0f;

    // Change lastFilterType from float to FilterType
    float lastFilterType = 0;

    // Add lastAmpEnvLevel member variable
    float lastAmpEnvLevel = 0.0f;

    // Add these member variables
    float lfoPhase = 0.0f;
    float lfoValue = 0.0f;
    float lastLfoPhase = 0.0f;  // Add this new member
    
    // Add this helper method
    float processLFO(float deltaTime);

    // // Helper functions to get properties with default values
    // float getProperty(const std::string& name) const {
    //     auto it = properties.find(name);
    //     return it != properties.end() ? it->second : 0.0f;
    // }

    std::vector<float> oscillatorOutput;

    float targetFrequency = 440.0f;
    float currentFrequency = 440.0f;
    float currentFreq1 = 440.0f;
    float currentFreq2 = 440.0f;
    float targetFreq1 = 440.0f;
    float targetFreq2 = 440.0f;
    float portamentoTime = 0.0f;
    float stateTime = 0.0f;  // Replace both noiseTime and portamentoStartTime

    uint32_t noise_counter = 0;  // Simple counter for noise

    void processBitcrusher(float* input, int numSamples, float bitcrushAmount, float sampleReduction);
};

#endif 