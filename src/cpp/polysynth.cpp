#include "polysynth.h"

PolySynth::PolySynth(float sampleRate, int maxVoices) 
    : sampleRate(sampleRate), maxVoices(maxVoices) {
    // Initialize voices
    
    voices.reserve(maxVoices);
    
    // Initialize sine wavetable
    std::vector<float> sineTable;
    sineTable.reserve(1024);  // Standard size for wavetable
    for (int i = 0; i < 1024; ++i) {
        sineTable.push_back(std::sin(2.0f * M_PI * i / 1024.0f));
    }
    wavetables[0] = sineTable;

    for (int i = 0; i < maxVoices; ++i) {
        voices.emplace_back(sampleRate, &wavetables);
        voices[i].setProperties(properties);  // Apply initial properties to each voice
        // voices[i].wavetables = &wavetables;  // Set the wavetables pointer for each voice
    }

    voiceCounter = 0;  // Initialize counter
}

void PolySynth::processBuffer(uintptr_t outputPtr, int bufferSize) {
    float* output = reinterpret_cast<float*>(outputPtr);
    std::fill(output, output + bufferSize * 2, 0.0f);
    frameCounter++;
    
    float monoBuffer[128];  // Assuming max buffer size of 128
    
    for (auto& voice : voices) {
        if (voice.isActive) {
            std::fill(monoBuffer, monoBuffer + bufferSize, 0.0f);
            voice.processBuffer(monoBuffer, bufferSize);
            
            // Apply the voice's fixed panning
            for (int i = 0; i < bufferSize; i++) {
                output[i * 2] += monoBuffer[i] * voice.gainLeft;     // Left
                output[i * 2 + 1] += monoBuffer[i] * voice.gainRight; // Right
            }
        }
    }

    float masterGain = properties["masterGain"];
    for (int i = 0; i < bufferSize * 2; i++) {
        output[i] *= masterGain;
    }
}

// float PolySynth::process() {
//     float output = 0.0f;
    
//     for (int i = 0; i < maxVoices; ++i) {
//         if (voices[i].isActive) {
//             output += voices[i].process();        
//         }
//     }
    
//     // if (activeVoices > 0) {
//     //     output *= 0.5f;
//     // }
    
//     return output;
// }

void PolySynth::noteOn(int m, float velocity) {
    int currentPolyphony = static_cast<int>(properties["polyphony"]);
    
    // First find and turn off any existing notes
    for (int i = 0; i < maxVoices; ++i) {
        auto& v = voices[i];
        if (v.midiNote == m && v.isActive) {
            v.abort();
        }
    }

    // Count ONLY active voices (excluding aborting ones)
    int activeCount = 0;
    for (int i = 0; i < maxVoices; ++i) {
        if (voices[i].isActive && !voices[i].isAborting) {
            activeCount++;
        }
    }

    // If we're at polyphony limit, find oldest voice and steal it
    if (activeCount >= currentPolyphony) {
        int oldestVoiceIndex = 0;
        int oldestStartTime = std::numeric_limits<int>::max();
        
        for (int i = 0; i < maxVoices; ++i) {
            auto& v = voices[i];
            if (v.isActive && !v.isAborting && v.startTime < oldestStartTime) {
                oldestStartTime = v.startTime;
                oldestVoiceIndex = i;
            }
        }
        
        voices[oldestVoiceIndex].abort();
    }

    // Find first available voice
    for (int i = 0; i < maxVoices; ++i) {
        auto& v = voices[i];
        if (!v.isActive && !v.isAborting) {
            v.setProperties(properties);
            
            // Calculate time-based panning using autoPanWidth and autoPanRate
            // float panTime = static_cast<float>(frameCounter) ;  // Convert to seconds
            //float pan = properties["autoPanWidth"] * std::sin(2.0f * M_PI * properties["autoPanRate"] * frameCounter/441.0f);
            float pan = properties["autoPanWidth"] * std::sin(2.0f * M_PI * properties["autoPanRate"] * voiceCounter/20.f);



            float angle = (pan + 1.0f) * M_PI / 4.0f;
            v.gainLeft = std::cos(angle);
            v.gainRight = std::sin(angle);
            
            v.noteOn(m, velocity, lastMidiNote);
            v.startTime = voiceCounter++;
            break;
        }
    }
    
    lastMidiNote = m;
}

void PolySynth::noteOff(int m) {
    for (int i = 0; i < maxVoices; ++i) {
        auto& v = voices[i];  // Changed to reference
        if (v.midiNote == m && v.isActive) {
            v.noteOff();
        }
    }
}


void PolySynth::setProperties(const std::map<std::string, float>& props) {
    // Update local properties
    for (const auto& [name, value] : props) {
        properties[name] = value;
    }
}

void PolySynth::loadWavetable(float key, const std::vector<float>& table) {
    wavetables[key] = table;
}