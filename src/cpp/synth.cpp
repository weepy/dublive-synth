#include "synth.h"
#include <cmath>
#include "filter_coefficients.h"

constexpr float TWO_PI = 6.28318530718f;

Synth::Synth(float sampleRate, std::map<float, std::vector<float>>* wavetables) 
    : sampleRate(sampleRate), wavetables(wavetables) {
    
    oscillatorOutput.resize(128);  // Pre-allocate the buffer

    wave1 = &(wavetables->begin()->second);
    wave2 = &(wavetables->begin()->second);
   
}

void Synth::processBuffer(float* buffer, int bufferSize) {
    float deltaTime = bufferSize / sampleRate;
    
    // Update portamento
    if (portamentoTime > 0.0f) {
        portamentoStartTime += deltaTime;
        float t = std::min(portamentoStartTime / portamentoTime, 1.0f);
        // Exponential interpolation for smoother frequency transitions
        currentFreq1 = currentFreq1 * std::pow(targetFreq1 / currentFreq1, t);
        currentFreq2 = currentFreq2 * std::pow(targetFreq2 / currentFreq2, t);
    }
    
    // Use the pre-calculated frequencies
    float freq1 = currentFreq1;
    float freq2 = currentFreq2;

    // Add LFO processing
    float lfoMod = processLFO(deltaTime);
    
    // Get base parameters before modulation
    float phaseOffset1 = properties["phaseOffset1"];
    float phaseOffset2 = properties["phaseOffset2"];
    float fmAmount = properties["fmAmount"];
    float mix = properties["mix"];
    float modulatedCutoff = properties["cutoff"];
    
    // Apply LFO based on destination
    int destination = static_cast<int>(properties["lfoDestination"]);
    switch(destination) {
        case 0: // Oscillators (affects both frequencies)
            freq1 *= (1.0f + lfoMod);
            freq2 *= (1.0f + lfoMod);
            break;
        case 1: // Filter
            modulatedCutoff += lfoMod;
            break;
        case 2: // FM
            fmAmount += lfoMod * fmAmount;
            break;
        case 3: // Phase
            phaseOffset1 += lfoMod;
            phaseOffset2 += lfoMod;
            break;
        case 4: // Mix
            mix += lfoMod;
            break;
    }

    float filterEnvLevel = filterEnv.process(deltaTime);
    modulatedCutoff += filterEnvLevel * properties["filterEnvAmount"];

    
    
    int phaseMode1 = int(properties["phaseMode1"]);
    int phaseMode2 = int(properties["phaseMode2"]);
    
    mix = std::clamp(mix, 0.0f, 1.0f);

    for (int i = 0; i < bufferSize; ++i) {
        float osc2 = processOscillator(wave2, phaseOffset2, phaseMode2, freq2, phase2);
        float phaseModulation = osc2 * fmAmount;
        float osc1 = processOscillator(wave1, phaseOffset1 + phaseModulation, phaseMode1, freq1, phase1);
        oscillatorOutput[i] = osc1 * mix + osc2 * (1.0f - mix);
    }

    // Process filter in-place
    processFilter(oscillatorOutput.data(), bufferSize, modulatedCutoff);
    

    float ampEnvLevel = ampEnv.process(deltaTime);
    float ampEnvIncrement = (ampEnvLevel - lastAmpEnvLevel) / bufferSize;
    
    float currentAmpEnv = lastAmpEnvLevel;
    for (int i = 0; i < bufferSize; ++i) {
        buffer[i] += oscillatorOutput[i] * currentAmpEnv * velocity;
        currentAmpEnv += ampEnvIncrement;
    }
    
    lastAmpEnvLevel = ampEnvLevel;
    

    // // Apply envelope and velocity
    // for (int i = 0; i < bufferSize; ++i) {
    //     buffer[i] += filtered[i] * ampEnvLevel * velocity;
    // }
    
    if(!ampEnv.isActive()) {
        isActive = false;
        isAborting = false;  // Clear the aborting flag when voice is completely inactive
    }
}

void Synth::processFilter(float* input, int numSamples, float cutoff01) {
    if(cutoff01 > 0.99f) {
        cutoff01 = 0.99f;
    }
    else if(cutoff01 < 0.001f) {
        cutoff01 = 0.001f;
    }

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
}

float Synth::processOscillator(const std::vector<float>* wave, float phaseOffset, int phaseMode, float freq, float& phase) {
    float output = 0.0f;
    int wavetableSize = wave->size();

    // Simply add the offset to the phase (phaseOffset should be in range [0, 1])
    // float phaseWithOffset = ;
    // Then wrap to [0, 1) range
    // float phaseWithOffset = std::fmod(phase + phaseOffset + 10.f, 1.0f);
    // if (phaseWithOffset < 0.0f) phaseWithOffset += 1.0f;

    // Process main waveform using phase with offset
    float position1 = phase * wavetableSize;
    size_t index1_1 = static_cast<size_t>(position1);
    size_t index1_2 = (index1_1 + 1) % wavetableSize;
    float frac1 = position1 - index1_1;
    float sample1 = (*wave)[index1_1] + frac1 * ((*wave)[index1_2] - (*wave)[index1_1]);

    // If phase modulation is enabled, process second waveform with offset
    if (phaseMode > 0) {
        // For the second sample, offset by 0.5
        float offsetPhase = std::fmod(phase + phaseOffset + 10.f, 1.0f);

        float position2 = offsetPhase * wavetableSize;
        size_t index2_1 = static_cast<size_t>(position2);
        size_t index2_2 = (index2_1 + 1) % wavetableSize;
        float frac2 = position2 - index2_1;
        float sample2 = (*wave)[index2_1] + frac2 * ((*wave)[index2_2] - (*wave)[index2_1]);

        // Apply phase modulation based on mode
        if (phaseMode == 1) {
            output = sample1 - sample2; // Subtraction mode
        } else if (phaseMode == 2) {
            output = sample1 * sample2; // Multiplication mode
        }
    } else {
        output = sample1; // No phase modulation
    }
    
    phase += freq / sampleRate;
    if (phase >= 1.0f) phase -= 1.0f;
    
    return output;
}

void Synth::noteOn(int m, float vel, int fromMidiNote) {
    midiNote = m;
    velocity = vel;
    isActive = true;
    isAborting = false;
    
    portamentoTime = properties["portamento"];
    portamentoStartTime = 0.0f;
    
    // Calculate target frequencies with offsets
    targetFreq1 = std::pow(2.0f, 
        (midiNote + properties["semi1"] + 
         properties["cent1"] / 100.0f + 
         properties["oct1"] * 12.0f) / 12.0f);
         
    targetFreq2 = std::pow(2.0f, 
        (midiNote + properties["semi2"] + 
         properties["cent2"] / 100.0f + 
         properties["oct2"] * 12.0f) / 12.0f);
    
    if (fromMidiNote >= 0 && portamentoTime > 0.0f) {
        // Set starting frequencies from the previous note
        currentFreq1 = std::pow(2.0f, 
            (fromMidiNote + properties["semi1"] + 
             properties["cent1"] / 100.0f + 
             properties["oct1"] * 12.0f) / 12.0f);
             
        currentFreq2 = std::pow(2.0f, 
            (fromMidiNote + properties["semi2"] + 
             properties["cent2"] / 100.0f + 
             properties["oct2"] * 12.0f) / 12.0f);
    } else {
        // No portamento, set current frequencies directly to target
        currentFreq1 = targetFreq1;
        currentFreq2 = targetFreq2;
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
    
    // baseCutoff = properties["cutoff"]; // Store base cutoff for filter envelope
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
        
        // Check if wavetable property was updated
        if (name == "wave1") {
            
            wave1 = &((*wavetables)[value]);
            
        }
        if (name == "wave2") {
            wave2 = &((*wavetables)[value]);
        }
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
    return lfoValue * properties["lfoAmount"]; // Amount is already 0-1
} 