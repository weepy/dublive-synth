#include "synth.h"
#include <cmath>
#include "filter_coefficients.h"

constexpr float TWO_PI = 6.28318530718f;

Synth::Synth(float sampleRate, std::map<float, std::vector<float>>* wavetables) 
    : sampleRate(sampleRate), wavetables(wavetables) {
    
    oscillatorOutput.resize(128);  // Pre-allocate the buffer

    wave1 = &(wavetables->begin()->second);
    wave2 = &(wavetables->begin()->second);
    wave3 = 0;
    // wave3 = &(wavetables->begin()->second);
   
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
        case 3: // Mix
            mix += lfoMod;
            break;
        case 4: // Phase
            phaseOffset1 += lfoMod;
            phaseOffset2 += lfoMod;
            break;
    }

    float filterEnvLevel = filterEnv.process(deltaTime);
    modulatedCutoff += filterEnvLevel * properties["filterEnvAmount"];

    
    
    int phaseMode1 = 0;//int(properties["phaseMode1"]);
    int phaseMode2 = 0;//int(properties["phaseMode2"]);
    
    mix = std::clamp(mix, 0.0f, 1.0f);

    
    
    float osc2Enabled = properties["osc2Enabled"];
    // float wave3Level = properties["wave3Level"];
    // float wave3Decay = properties["wave3Decay"] * properties["wave3Decay"] * properties["wave3Decay"] * 10.0f;

    
    // Process main oscillators
    for (int i = 0; i < bufferSize; ++i) {
        float osc2 = 0.0f;
        
        float sample;
        float modulated_freq1 = freq1;

        // Only process osc2 if enabled
        if (osc2Enabled > 0.5f) {
            osc2 = processOscillator(wave2, phaseOffset2, phaseMode2, freq2, phase2);
            modulated_freq1 *= (1.0f + osc2 * fmAmount);
        }
        
        float osc1 = processOscillator(wave1, phaseOffset1, phaseMode1, modulated_freq1, phase1);
        

        sample = osc1 * (1.0f - mix) + osc2 * mix;// * wave3Level;

        // // Add wave3 processing        
        // if (wave3 && wave3Level > 0.0f && wave3Playing) {
        //     size_t index1 = static_cast<size_t>(pos3);
        //     size_t index2 = (index1 + 1);// % wave3->size();  // Wrap around to start if needed

            
        //     float frac = pos3 - index1;

        //     float l = (*wave3)[index1]; 
        //     float r = (*wave3)[index2];
            
        //     float wave3Sample = l + frac * (r - l);
            
        //     float osc3 = wave3Sample;
        //     float sampleStep = std::pow(2.0f, (midiNote - 72) / 12.0f);
           
        //     pos3 += sampleStep;

        //     float fadeOutTime = wave3->size() * wave3Decay / sampleRate;

        //     float env =  1.0f - (stateTime / fadeOutTime); // * sampleRate / wave3Decay);

        //     if(env < 0.0f) {
        //         env = 0.0f;
        //         wave3Playing = false;
        //     }
        //     sample += osc3 * wave3Level * env;
        // }

        // Mix oscillators (osc2 will be 0 if disabled)
        oscillatorOutput[i] = sample;

        // printf("%f\n", pos3);
        
    }
    
    // float noiseEnabled = properties["noiseEnabled"];
    float noiseLevel = properties["noiseLevel"];

    // Add noise if enabled
    if (noiseLevel != 0.0f) {
        
        float noiseDecay = properties["noiseDecay"];
        float noiseColor = properties["noiseColor"];

        noiseColor*=noiseColor;
        noiseDecay*=noiseDecay*noiseDecay*10.f;
        noiseLevel*=noiseLevel;

        // Calculate current noise amplitude using exponential decay
        float currentNoiseAmplitude = noiseDecay == 10.f ? 1.f : std::exp(-stateTime / noiseDecay);
        
        // Only process noise if the envelope hasn't fully decayed
        if (currentNoiseAmplitude > 0.001f) {  // Small threshold to avoid processing tiny values
            for (int i = 0; i < bufferSize; ++i) {
                noise_counter = noise_counter * 1664525 + 1013904223;
                noiseValue = (float)(noise_counter) * (2.0f / (float)4294967295UL) - 1.0f;
                noiseLowpassOutput = noiseLowpassOutput * (1.0f - noiseColor) + noiseValue * noiseColor;
                
                oscillatorOutput[i] += noiseLowpassOutput * noiseLevel * currentNoiseAmplitude;
            }
        }
    }
    
    // Process bitcrusher before filter
    float bitcrushAmount = properties["bitcrushAmount"];
    float sampleReduction = properties["sampleReduction"];
    
    if (bitcrushAmount > 0.0f || sampleReduction > 0.0f) {
        processBitcrusher(oscillatorOutput.data(), bufferSize, bitcrushAmount, sampleReduction);
    }
    
    // Process filter after bitcrusher
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

float Synth::processOscillator(const std::vector<float>* wave, float phaseOffset, int phaseMode, float freq, float& phase) {
    float output = 0.0f;
    int wavetableSize = wave->size();
    
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
    
    // Simple phase increment (multiplier is now applied to freq)
    phase += freq / sampleRate;
    if (phase >= 1.0f) phase -= 1.0f;
    
    return output;
}

void Synth::noteOn(int m, float vel, int fromMidiNote) {
    midiNote = m;
    velocity = vel;
    isActive = true;
    isAborting = false;
    
    stateTime = 0.0f;
    portamentoTime = properties["portamento"];
    
    // Reset LFO phase if retrigger is enabled
    if (properties["lfoRetrigger"] > 0.5f) {
        lfoPhase = 0.0f;
        lfoValue = 0.0f;
        lastLfoPhase = 0.0f;
    }
    
    // Calculate the base wavetable size (1380)
    const int BASE_WAVETABLE_SIZE = 1380;
    
    // Calculate multipliers for each wave
    int multiplier1 = std::round(static_cast<float>(wave1->size()) / BASE_WAVETABLE_SIZE);
    int multiplier2 = std::round(static_cast<float>(wave2->size()) / BASE_WAVETABLE_SIZE);
    if (multiplier1 < 1) multiplier1 = 1;
    if (multiplier2 < 1) multiplier2 = 1;
    
    // Modified frequency calculations using standard MIDI tuning (A4 = 440Hz)
    targetFreq1 = (440.0f * std::pow(2.0f, 
        (midiNote - 69 + properties["semi1"] + 
         properties["cent1"] / 100.0f + 
         properties["oct1"] * 12.0f) / 12.0f)) / multiplier1;
         
    targetFreq2 = (440.0f * std::pow(2.0f, 
        (midiNote - 69 + properties["semi2"] + 
         properties["cent2"] / 100.0f + 
         properties["oct2"] * 12.0f) / 12.0f)) / multiplier2;
    
    if (fromMidiNote >= 0 && portamentoTime > 0.0f) {
        // Set starting frequencies from the previous note using same formula
        currentFreq1 = (440.0f * std::pow(2.0f, 
            (fromMidiNote - 69 + properties["semi1"] + 
             properties["cent1"] / 100.0f + 
             properties["oct1"] * 12.0f) / 12.0f)) / multiplier1;
             
        currentFreq2 = (440.0f * std::pow(2.0f, 
            (fromMidiNote - 69 + properties["semi2"] + 
             properties["cent2"] / 100.0f + 
             properties["oct2"] * 12.0f) / 12.0f)) / multiplier2;
    } else {
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
        
        // Check if wavetable property was updated
        if (name == "wave1") {
            
            wave1 = &((*wavetables)[value]);
            
        }
        if (name == "wave2") {
            wave2 = &((*wavetables)[value]);
        }
        if (name == "wave3") {
            wave3 = &((*wavetables)[value]);
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