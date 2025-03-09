#include "synth.h"
#include <cmath>
#include "filter_coefficients.h"

constexpr float TWO_PI = 6.28318530718f;

Synth::Synth(float sampleRate, std::map<float, std::map<int, std::vector<float>>>* wavetables) 
    : sampleRate(sampleRate) {
    
    oscillatorOutput.resize(128);  // Pre-allocate the buffer

    properties["filterKeyTracking"] = 1.0f;  // Add default value for key tracking
    
    // Initialize frequency variables
    currentFreq1 = 261.63f;
    currentFreq2 = 261.63f;
    currentFreq3 = 261.63f;
    targetFreq1 = 261.63f;
    targetFreq2 = 261.63f;
    targetFreq3 = 261.63f;
}

void Synth::processBuffer(float* buffer, int bufferSize) {
    float deltaTime = bufferSize / sampleRate;
    stateTime += deltaTime;  // Update state time for every buffer
    
    // Update portamento
    if (portamentoTime > 0.0f) {
        float t = std::min(stateTime / portamentoTime, 1.0f);
        // Exponential interpolation for smoother frequency transitions
        currentFreq1 = currentFreq1 * std::pow(targetFreq1 / currentFreq1, t);
        currentFreq2 = currentFreq2 * std::pow(targetFreq2 / currentFreq2, t);
        currentFreq3 = currentFreq3 * std::pow(targetFreq3 / currentFreq3, t);
    }
    
    // Use the pre-calculated frequencies
    float freq1 = currentFreq1;
    float freq2 = currentFreq2;
    float freq3 = currentFreq3;

    // Add LFO processing
    float lfoMod = processLFO(deltaTime);
    
    // Get base parameters before modulation
    float fmAmount = properties["fmAmount"];
    float mix = properties["mix"];
    float modulatedCutoff = properties["cutoff"];
    
    // Add keyboard tracking
    float keyboardTracking = properties["filterKeyTracking"];
    float noteOffset = (midiNote - 69) * keyboardTracking; // A4 (MIDI note 69) is the reference note
    // Apply keyboard tracking to the linear cutoff parameter first
    modulatedCutoff = modulatedCutoff + (noteOffset / 120.0f);
    
    // Apply LFO based on destination
    int destination = static_cast<int>(properties["lfoDestination"]);
    switch(destination) {
        case 0: // Oscillators (affects both frequencies)
            freq1 *= (1.0f + lfoMod);
            freq2 *= (1.0f + lfoMod);
            freq3 *= (1.0f + lfoMod);
            break;
        case 1: // Filter
            modulatedCutoff += lfoMod;
            break;
        case 2: // FM
            fmAmount += lfoMod * fmAmount;
            break;
        case 3: // Mix
            mix += lfoMod;
            break;
    }

    float filterEnvLevel = filterEnv.process(deltaTime);
    modulatedCutoff += filterEnvLevel * properties["filterEnvAmount"];
    
    mix = std::clamp(mix, 0.0f, 1.0f);
    
    bool osc2Enabled = properties["osc2Enabled"] > 0.5f;
    
    
    // Process main oscillators - branch based on osc2Enabled
    if (osc2Enabled && currentWavetable2 && currentWavetable1) {
        // Cache wavetable data and size outside the loop
        const float* wavetable1Data = currentWavetable1->data();
        int wavetable1Size = currentWavetable1->size();
        const float* wavetable2Data = currentWavetable2->data();
        int wavetable2Size = currentWavetable2->size();
        
        // Both oscillators enabled
        for (int i = 0; i < bufferSize; ++i) {
            // Process osc2 first for FM
            float osc2 = processOscillator(wavetable2Data, wavetable2Size, freq2, pos2, isLooping2);
            
            // Apply FM from osc2 to osc1
            float modulated_freq1 = freq1 * (1.0f + osc2 * fmAmount);
            
            // Process osc1 with modulated frequency
            float osc1 = processOscillator(wavetable1Data, wavetable1Size, modulated_freq1, pos1, isLooping1);
            
            // Mix the oscillators
            oscillatorOutput[i] = osc1 * (1.0f - mix) + osc2 * mix;
        }
    } else if (currentWavetable1) {
        // Cache wavetable data and size outside the loop
        const float* wavetable1Data = currentWavetable1->data();
        int wavetable1Size = currentWavetable1->size();
        
        // Only osc1 enabled
        for (int i = 0; i < bufferSize; ++i) {
            oscillatorOutput[i] = processOscillator(wavetable1Data, wavetable1Size, freq1, pos1, isLooping1);
        }
    } else {
        // No oscillators enabled
        for (int i = 0; i < bufferSize; ++i) {
            oscillatorOutput[i] = 0.0f;
        }
    }
    
    // Process wavetable3 if enabled (replacing noise)
    float wave3Level = properties["wave3Level"];
    if (properties["osc3Enabled"] > 0.5f && wave3Level > 0.0f && currentWavetable3 && wave3Playing) {
        float wave3Decay = properties["wave3Decay"];
        
        // Square the parameters for more intuitive control
        wave3Decay *= wave3Decay * wave3Decay * 10.f;
        wave3Level *= wave3Level;

        // Calculate current amplitude using exponential decay
        float currentWave3Amplitude = wave3Decay == 10.f ? 1.f : std::exp(-stateTime / wave3Decay);
        
        // Only process if the envelope hasn't fully decayed
        if (currentWave3Amplitude > 0.001f) {  // Small threshold to avoid processing tiny values
            // Cache wavetable data and size outside the loop
            const float* wavetable3Data = currentWavetable3->data();
            int wavetable3Size = currentWavetable3->size();
            
            for (int i = 0; i < bufferSize; ++i) {
                float osc3 = processOscillator(wavetable3Data, wavetable3Size, freq3, pos3, isLooping3);
                oscillatorOutput[i] += osc3 * wave3Level * currentWave3Amplitude;
            }
            
            // If one-shot mode and we've reached the end, mark as not playing
            if (!isLooping3 && pos3 >= currentWavetable3->size()) {
                wave3Playing = false;
            }
        } else {
            // If amplitude has decayed below threshold, mark as not playing
            wave3Playing = false;
        }
    }
    
    processDistortion(oscillatorOutput.data(), bufferSize, properties["distortion"], 0);
    // Process filter after bitcrusher
    processFilter(oscillatorOutput.data(), bufferSize, modulatedCutoff);
    
    // Apply amplitude envelope
    float ampEnvLevel = ampEnv.process(deltaTime);
    float ampEnvIncrement = (ampEnvLevel - lastAmpEnvLevel) / bufferSize;
    
    float currentAmpEnv = lastAmpEnvLevel;
    for (int i = 0; i < bufferSize; ++i) {
        buffer[i] += oscillatorOutput[i] * currentAmpEnv * velocity;
        currentAmpEnv += ampEnvIncrement;
    }
    
    lastAmpEnvLevel = ampEnvLevel;
    
    if(!ampEnv.isActive()) {
        isActive = false;
        isAborting = false;  // Clear the aborting flag when voice is completely inactive
    }
}

void Synth::processFilter(float* input, int numSamples, float cutoff01) {
    cutoff01 = std::clamp(cutoff01, 0.001f, 0.99f);

    float cutoff = 40.0f * std::pow(sampleRate / (160.0f), cutoff01);

    float resonance = properties["resonance"];
    float filterType = properties["filterType"];
    
    // Update coefficients if needed
    if (cutoff != lastCutoff || resonance != lastResonance || filterType != lastFilterType) {
        auto coeffs = calculateBiquadCoefficients(cutoff, sampleRate, resonance, filterType);
        b0 = coeffs.b0;
        b1 = coeffs.b1;
        b2 = coeffs.b2;
        a1 = coeffs.a1;
        a2 = coeffs.a2;
        
        lastCutoff = cutoff;
        lastResonance = resonance;
        lastFilterType = filterType;
    }
    
    // Process all samples in-place
    float x0;
    for (int i = 0; i < numSamples; ++i) {
        x0 = input[i];
        input[i] = b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
        
        // Update filter state
        x2 = x1;
        x1 = x0;
        y2 = y1;
        y1 = input[i];
    }

    // Add second pass for Lowpass 24
    if (static_cast<int>(filterType) == 0) {  // Lowpass 24
        // Use second set of state variables for the second pass
        for (int i = 0; i < numSamples; ++i) {
            x0 = input[i];
            input[i] = b0 * x0 + b1 * x1_2 + b2 * x2_2 - a1 * y1_2 - a2 * y2_2;
            
            // Update second filter state
            x2_2 = x1_2;
            x1_2 = x0;
            y2_2 = y1_2;
            y1_2 = input[i];
        }
    }
}

float Synth::processOscillator(const float* wavetableData, int wavetableSize, float freq, float& pos, bool shouldLoop) {
    // if (!wavetableData || wavetableSize <= 0) return 0.0f;
    
    // Calculate playback rate relative to C4 (MIDI note 60)
    // For C4, we want to play at original speed (1 sample per step)
    // For other notes, we scale accordingly
    float playbackRate = freq / 261.63f; // 261.63 Hz is C4
    
    // Update position (no longer using phase)
    pos += playbackRate;
    
    // Handle looping or one-shot behavior
    if (pos >= wavetableSize) {
        if (shouldLoop) {
            // Loop back to the beginning
            pos -= wavetableSize;
        } else {
            // One-shot mode: stay at the end and return 0
            pos = wavetableSize;
            return 0.0f;
        }
    }
    
    // Get integer position and fractional part for interpolation
    int index1 = static_cast<int>(pos);
    int index2 = (index1 + 1) % wavetableSize;
    float frac = pos - index1;
    
    // Linear interpolation between samples
    float sample = wavetableData[index1] + frac * (wavetableData[index2] - wavetableData[index1]);
    
    return sample;
}

float Synth::calculateFrequency(int midiNote, float semi, float cent, float oct, float tune) {
    // Special case: if tune is -999, return fixed frequency of 261.63 Hz (C4)
    if (tune == -999.0f) {
        return 261.63f;
    }
    
    // Normal calculation
    return 261.63f * std::pow(2.0f, 
        (midiNote - 60 + semi + 
         cent / 100.0f + 
         oct * 12.0f + 
         tune) / 12.0f);
}

void Synth::noteOn(int m, float vel, int fromMidiNote) {
    midiNote = m;
    velocity = vel;
    isActive = true;
    isAborting = false;
    
    stateTime = 0.0f;
    portamentoTime = properties["portamento"];
    portamentoTime*=portamentoTime*10.f;
    
    
    // Reset oscillator positions to beginning of sample
    pos1 = 0.0f;
    pos2 = 0.0f;
    pos3 = 0.0f;
    
    // Set the loop flags based on properties
    isLooping1 = properties["loop1"] > 0.5f;
    isLooping2 = properties["loop2"] > 0.5f;
    isLooping3 = properties["loop3"] > 0.5f;
    
    // Reset LFO phase if retrigger is enabled
    if (properties["lfoRetrigger"] > 0.5f) {
        lfoPhase = 0.0f;
        lfoValue = 0.0f;
        lastLfoPhase = 0.0f;
    }
    
    // Calculate frequencies using the new helper function
    targetFreq1 = calculateFrequency(midiNote, 
                                    properties["semi1"], 
                                    properties["cent1"], 
                                    properties["oct1"], 
                                    properties["tune1"]);
    
    targetFreq2 = calculateFrequency(midiNote, 
                                    properties["semi2"], 
                                    properties["cent2"], 
                                    properties["oct2"], 
                                    properties["tune2"]);
    
    targetFreq3 = calculateFrequency(midiNote, 
                                    properties["semi3"], 
                                    properties["cent3"], 
                                    properties["oct3"], 
                                    properties["tune3"]);
    
    if (fromMidiNote >= 0 && portamentoTime > 0.0f) {
        // Set starting frequencies from the previous note
        currentFreq1 = calculateFrequency(fromMidiNote, 
                                         properties["semi1"], 
                                         properties["cent1"], 
                                         properties["oct1"], 
                                         properties["tune1"]);
        
        currentFreq2 = calculateFrequency(fromMidiNote, 
                                         properties["semi2"], 
                                         properties["cent2"], 
                                         properties["oct2"], 
                                         properties["tune2"]);
        
        currentFreq3 = calculateFrequency(fromMidiNote, 
                                         properties["semi3"], 
                                         properties["cent3"], 
                                         properties["oct3"], 
                                         properties["tune3"]);
    } else {
        currentFreq1 = targetFreq1;
        currentFreq2 = targetFreq2;
        currentFreq3 = targetFreq3;
    }
    
    // Update envelope parameters and trigger
    ampEnv.setParameters(
        properties["ampAttack"],
        properties["ampDecay"],
        properties["ampSustain"],
        properties["ampRelease"]
    );
    
    filterEnv.setParameters(
        properties["filterAttack"],
        properties["filterDecay"],
        properties["filterSustain"],
        properties["filterRelease"]
    );
    
    ampEnv.noteOn();
    filterEnv.noteOn();
    
    wave3Playing = true;  // Reset wave3 playback
    pos3 = 0.0f;       // Reset pos3
}

void Synth::noteOff() {
    ampEnv.noteOff();
    filterEnv.noteOff();
}

void Synth::abort() {
    ampEnv.abort();
    isAborting = true;  // Set the aborting flag
}

void Synth::setProperties(const std::map<std::string, float>& props) {
    for (const auto& [name, value] : props) {
        properties[name] = value;
        
        // Remove the wavetable lookup code since it's now handled by PolySynth
        // if (name == "wave1") {
        //     auto it = wavetables->find(value);
        //     if (it != wavetables->end()) {
        //         currentWave1 = &(it->second);
        //     }
        // }
        // if (name == "wave2") {
        //     auto it = wavetables->find(value);
        //     if (it != wavetables->end()) {
        //         currentWave2 = &(it->second);
        //     }
        // }
    }
}

float Synth::processLFO(float deltaTime) {
    float rate = properties["lfoRate"]; // Already in 0-1 range
    float frequency;
    
    if (properties["lfoSync"] > 0.5f) {
        // Sync to note frequency - rate becomes a multiplier/divider
        frequency = this->frequency * rate;
    } else {
        // Free running - rate goes from 0.1 Hz to 20 Hz
        frequency = 0.1f * std::pow(200.0f, rate);
    }

    lfoPhase += frequency * deltaTime;
    if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

    float waveform = properties["lfoWaveform"];
    
    // Generate LFO waveform
    if (waveform < 1.0f) { // Triangle
        lfoValue = (lfoPhase < 0.5f) ? 
            4.0f * lfoPhase - 1.0f : 
            3.0f - 4.0f * lfoPhase;
    } else if (waveform < 2.0f) { // Sawtooth
        lfoValue = 2.0f * lfoPhase - 1.0f;
    } else if (waveform < 3.0f) { // Square
        lfoValue = (lfoPhase < 0.5f) ? 1.0f : -1.0f;
    } else if (waveform < 4.0f) { // Sample and Hold
        if (lfoPhase < lastLfoPhase) {
            lfoValue = 2.0f * (float)rand() / (float)RAND_MAX - 1.0f;
        }
    } else { // Sine
        lfoValue = std::sin(TWO_PI * lfoPhase);
    }
    
    lastLfoPhase = lfoPhase;

    // Apply fade-in
    float fadeInTime = properties["lfoFadeIn"];
    float fadeInMultiplier = 1.0f;
    if (fadeInTime > 0.0f) {
        fadeInMultiplier = std::min(stateTime / fadeInTime, 1.0f);
    }
    
    return lfoValue * properties["lfoAmount"] * fadeInMultiplier;
}

void Synth::processBitcrusher(float* input, int numSamples, float bitcrushAmount, float sampleReduction) {
    // Bit depth reduction (1.0 = 1 bit, 0.0 = full bit depth)
    float levels = std::pow(2.0f, std::floor((1.0f - bitcrushAmount) * 16.0f));
    
    // Sample rate reduction (decimation)
    int skipSamples = std::max(1, static_cast<int>(sampleReduction * 50.0f));
    float holdValue = 0.0f;
    
    for (int i = 0; i < numSamples; ++i) {
        // Sample rate reduction
        if (i % skipSamples == 0) {
            holdValue = input[i];
        } else {
            input[i] = holdValue;
        }
        
        // Bit depth reduction
        if (bitcrushAmount > 0.0f) {
            input[i] = std::floor(input[i] * levels) / levels;
        }
    }
}

void Synth::processDistortion(float* input, int numSamples, float amount, float character) {
    // Pre-calculate gain and attenuation to avoid repeated calculations
    float gain = 1.0f + amount * 8.0f;  // Boost input signal
    float attenuation = 1.0f / (1.0f + amount);  // Output attenuation factor
    
    // Determine distortion type based on character parameter
    // character: 0.0-0.33 = hard clip, 0.33-0.66 = soft clip, 0.66-1.0 = wavefolder
    for (int i = 0; i < numSamples; i++) {
        float x = input[i] ;
        float p;
        
        // if (character < 0.33f) {
            // Hard clipping
            p = std::clamp(x* gain, -1.0f, 1.0f);
        // } 
        // else if (character < 0.66f) {
        //     // Soft clipping (x / (1 + |x|))
            // p = x / (1.0f + std::abs(x));
        // }
      
        
        input[i] = amount * p * attenuation + (1.0f - amount) * x;
    }
}
