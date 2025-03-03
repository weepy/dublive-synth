#include "filter_coefficients.h"
#include <cmath>

constexpr float TWO_PI = 6.28318530718f;

BiquadCoefficients calculateBiquadCoefficients(
    float cutoff,
    float sampleRate,
    float resonance,
    float type
) {
    BiquadCoefficients coeff;
    
    float w0 = TWO_PI * (cutoff / sampleRate);
    float cosw0 = std::cos(w0);
    float sinw0 = std::sin(w0);
    float alpha = sinw0 / (2.0f * (1.0f + resonance));
    float a0 = 1.0f + alpha;

    int filterType = static_cast<int>(type);
    switch (filterType) {
        case 0:  // Lowpass (was FilterType::Lowpass)
            coeff.b0 = ((1.0f - cosw0) * 0.5f) / a0;
            coeff.b1 = (1.0f - cosw0) / a0;
            coeff.b2 = coeff.b0;
            coeff.a1 = (-2.0f * cosw0) / a0;
            coeff.a2 = (1.0f - alpha) / a0;
            break;

        case 1:  // Highpass (was FilterType::Highpass)
            coeff.b0 = ((1.0f + cosw0) * 0.5f) / a0;
            coeff.b1 = -(1.0f + cosw0) / a0;
            coeff.b2 = coeff.b0;
            coeff.a1 = (-2.0f * cosw0) / a0;
            coeff.a2 = (1.0f - alpha) / a0;
            break;

        case 2:  // Bandpass (was FilterType::Bandpass)
            coeff.b0 = alpha / a0;
            coeff.b1 = 0.0f;
            coeff.b2 = -alpha / a0;
            coeff.a1 = (-2.0f * cosw0) / a0;
            coeff.a2 = (1.0f - alpha) / a0;
            break;

        case 3:  // Notch (was FilterType::Notch)
            coeff.b0 = 1.0f / a0;
            coeff.b1 = (-2.0f * cosw0) / a0;
            coeff.b2 = 1.0f / a0;
            coeff.a1 = (-2.0f * cosw0) / a0;
            coeff.a2 = (1.0f - alpha) / a0;
            break;
    }

    return coeff;
} 