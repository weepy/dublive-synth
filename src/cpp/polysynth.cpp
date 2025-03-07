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
    loadWavetable(0, sineTable);  // Now uses the mipmap-enabled loadWavetable

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
            
            // Calculate autopan
            float pan = properties["autoPanWidth"] * std::sin(2.0f * M_PI * properties["autoPanRate"] * voiceCounter/20.f);
            float angle = (pan + 1.0f) * M_PI / 4.0f;
            v.gainLeft = std::cos(angle);
            v.gainRight = std::sin(angle);

            // Calculate appropriate mip levels before note on
            float baseFreq = 440.0f * std::pow(2.0f, (m - 69) / 12.0f);
            
            // Calculate frequencies for all three oscillators
            float freq1 = baseFreq * std::pow(2.0f, 
                (properties["semi1"] + properties["cent1"] / 100.0f + 
                 properties["oct1"] * 12.0f) / 12.0f);
            float freq2 = baseFreq * std::pow(2.0f, 
                (properties["semi2"] + properties["cent2"] / 100.0f + 
                 properties["oct2"] * 12.0f) / 12.0f);
            float freq3 = baseFreq * std::pow(2.0f, 
                (properties["semi3"] + properties["cent3"] / 100.0f + 
                 properties["oct3"] * 12.0f) / 12.0f);
            
            // Select appropriate wavetables
            float wave1Key = properties["wave1"];
            float wave2Key = properties["wave2"];
            float wave3Key = properties["wave3"];
            
            if (auto it1 = wavetables.find(wave1Key); it1 != wavetables.end()) {
                v.setWavetable1(&(it1->second.at(0)));
            }
            
            if (auto it2 = wavetables.find(wave2Key); it2 != wavetables.end()) {
                v.setWavetable2(&(it2->second.at(0)));
            }
            
            if (auto it3 = wavetables.find(wave3Key); it3 != wavetables.end()) {
                v.setWavetable3(&(it3->second.at(0)));
            }
            
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
    // Create base wavetable and mipmaps
    std::map<int, std::vector<float>> mipLevels;
    mipLevels[0] = table;  // Original table
    
    // Create progressively downsampled versions
    std::vector<float> currentTable = table;
    int level = 1;
    
    // // Calculate how many mip levels we need for the full MIDI range
    // // For highest note (MIDI 127, ~12.5kHz), we need enough mip levels
    // // float highestFreq = 440.0f * std::pow(2.0f, (127 - 69) / 12.0f);  // Frequency of highest MIDI note
    // // float nyquist = sampleRate * 0.5f;
    // // int requiredMipLevels = std::ceil(std::log2(highestFreq / (nyquist / table.size()))) + 1;
    // int maxLevel = 1;// std::max(10, requiredMipLevels);  // Ensure we have enough mip levels

    // while (currentTable.size() > 4 && level < maxLevel)  {  // Minimum size threshold
    //     std::vector<float> downsampled;
    //     downsampled.reserve(currentTable.size() / 2);
        
    //     // Simple averaging downsample
    //     for (size_t i = 0; i < currentTable.size(); i += 2) {
    //         float avg = (currentTable[i] + currentTable[(i + 1) % currentTable.size()]) * 0.5f;
    //         downsampled.push_back(avg);
    //     }
        
    //     mipLevels[level] = downsampled;
    //     currentTable = downsampled;
    //     level++;
    // }
    
    wavetables[key] = mipLevels;
}