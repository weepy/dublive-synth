export const propertyDescriptors = [
    // OSC 1
    {
        name: 'wave1',
        value: 0,
        type: 'select',
        group: 'osc1',
        options: ['Sine', 'Square', 'Saw']
    },
    {
        name: 'oct1',
        value: 0,
        type: "number",
        group: 'osc1',
        step: 1,
    },
    {
        name: 'semi1',
        type: "number",
        value: 0,
        group: 'osc1',
        min: -12,
        max: 12,
        step: 1,
    },
    {
        name: 'cent1',
        type: "number",
        value: 0,
        group: 'osc1',
        min: -100,
        max: 100,
        step: 1,
    },
    {
        name: 'phaseOffset1',
        value: 0,
        min: 0,
        max: 1,
        step: 0.001,
        group: 'osc1',
        type: 'range'
    },
    {
        name: 'phaseMode1',
        value: 0,
        type: 'select',
        group: 'osc1',
        options: ['Off', 'Subtraction', 'Multiplication']
    },

    // OSC 2
    {
        name: 'wave2',
        value: 0,
        type: 'select',
        group: 'osc2',
        options: ['Sine', 'Square', 'Saw']
    },
    {
        name: 'oct2',
        value: 0,
        type: "number",
        group: 'osc2',
        step: 1,
    },
    {
        name: 'semi2',
        type: "number",
        value: 0,
        group: 'osc2',
        min: -12,
        max: 12,
        step: 1,
    },
    {
        name: 'cent2',
        type: "number",
        value: 0,
        group: 'osc2',
        min: -100,
        max: 100,
        step: 1,
    },
    {
        name: 'phaseOffset2',
        value: 0,
        min: 0,
        max: 1,
        step: 0.001,
        group: 'osc2',
        type: 'range'
    },
    {
        name: 'phaseMode2',
        value: 0,
        type: 'select',
        group: 'osc2',
        options: ['Off', 'Subtraction', 'Multiplication']
    },

    // Mix section
    {
        name: 'mix',
        value: 0.5,
        min: 0,
        max: 1,
        step: 0.001,    
        type: 'range',
        group: 'mix'
    },
    {
        name: 'fmAmount',
        value: 0,
        min: 0,
        max: 1,
        step: 0.01,
        type: 'range',
        group: 'mix'
    },

    // Filter section
    {
        name: 'cutoff',
        value: 1,
        min: 0,
        max: 1,
        step: 0.01,
        type: 'range',
        group: 'filter'
    },
    {
        name: 'resonance',
        value: 0.7,
        min: 0.1,
        max: 10,
        step: 0.01,
        type: 'range',
        group: 'filter'
    },

    // Filter Envelope
    {
        name: 'filterAttack',
        value: 0.1,
        min: 0,
        max: 2,
        step: 0.01,
        type: 'range',
        group: 'filterEnv'
    },
    {
        name: 'filterDecay',
        value: 0.1,
        min: 0,
        max: 2,
        step: 0.01,
        type: 'range',
        group: 'filterEnv'
    },
    {
        name: 'filterSustain',
        value: 0.7,
        min: 0,
        max: 1,
        step: 0.01,
        type: 'range',
        group: 'filterEnv'
    },
    {
        name: 'filterRelease',
        value: 0.1,
        min: 0,
        max: 2,
        step: 0.01,
        type: 'range',
        group: 'filterEnv'
    },
    {
        name: 'filterEnvAmount',
        value: 0.5,
        min: -1,
        max: 1,
        step: 0.01,
        type: 'range',
        group: 'filterEnv'
    },

    // Amp Envelope
    {
        name: 'ampAttack',
        value: 0.01,
        min: 0,
        max: 2,
        step: 0.01,
        type: 'range',
        group: 'ampEnv'
    },
    {
        name: 'ampDecay',
        value: 0.1,
        min: 0,
        max: 2,
        step: 0.01,
        type: 'range',
        group: 'ampEnv'
    },
    {
        name: 'ampSustain',
        value: 0.7,
        min: 0,
        max: 1,
        step: 0.01,
        type: 'range',
        group: 'ampEnv'
    },
    {
        name: 'ampRelease',
        value: 0.1,
        min: 0,
        max: 2,
        step: 0.01,
        type: 'range',
        group: 'ampEnv'
    },

    // LFO
    {
        name: "lfoWaveform",
        type: "select",
        value: 0,
        group: 'lfo',
        options: ["Triangle", "Sawtooth", "Square", "Sample & Hold", "Sine"]
    },
    {
        name: "lfoRate",
        type: "range",
        value: 0.5,
        min: 0,
        max: 1,
        step: 0.01,
        group: 'lfo'
    },
    {
        name: "lfoAmount",
        type: "range",
        value: 0,
        min: 0,
        max: 1,
        step: 0.01,
        group: 'lfo'
    },
    {
        name: "lfoDestination",
        type: "select",
        value: 0,
        group: 'lfo',
        options: ["Oscillators", "Filter", "FM", "Phase", "Mix"]
    },

    // Master
    {
        name: 'portamento',
        value: 0,
        min: 0,
        max: 2,
        step: 0.01,
        type: 'range',
        group: 'master'
    },
    {
        name: 'masterGain',
        value: 0.5,
        min: 0,
        max: 1,
        step: 0.01,
        type: 'range',
        group: 'master'
    },
    {
        name: 'polyphony',
        value: 4,
        min: 1,
        max: 8,
        step: 1,
        type: 'number',
        group: 'master'
    }
];
