#pragma once

struct BiquadCoefficients {
    float b0, b1, b2;
    float a1, a2;
};

// enum class FilterType {
//     Lowpass,
//     Highpass,
//     Bandpass,
//     Notch
// };

BiquadCoefficients calculateBiquadCoefficients(
    float cutoff,
    float sampleRate,
    float resonance,
    float type
); 