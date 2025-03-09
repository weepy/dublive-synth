import {waves} from './waves';

export const propertyDescriptors = [
    {
        name: 'name',
        value: 'Default',
        type: 'string'
    },
    // OSC 1
    {
        name: 'wave1',
        value: waves[0],
        type: 'wave',
        options: waves
    },
    {
        name: 'oct1',
        value: 0,
        type: "number",
        step: 1,
    },
    {
        name: 'semi1',
        type: "number",
        value: 0,
        min: -12,
        max: 12,
        step: 1,
    },
    {
        name: 'cent1',
        type: "number",
        value: 0,
        min: -100,
        max: 100,
        step: 1,
    },
    // {
    //     name: 'phaseOffset1',
    //     value: 0,
    //     min: 0,
    //     max: 1,
    //     step: 0.001,
    //     group: 'osc1',
    //     type: 'range'
    // },
    // {
    //     name: 'phaseMode1',
    //     value: 0,
    //     type: 'select',
    //     group: 'osc1',
    //     options: ['Off', 'Subtraction', 'Multiplication']
    // },

    // OSC 2
    {
        name: 'wave2',
        value: waves[0],
        type: 'wave',
        options: waves
    },
    {
        name: 'oct2',
        value: 0,
        type: "number",
        step: 1,
    },
    {
        name: 'semi2',
        type: "number",
        value: 0,
        min: -12,
        max: 12,
        step: 1,
    },
    {
        name: 'cent2',
        type: "number",
        value: 0,
        min: -100,
        max: 100,
        step: 1,
    },
    {
        name: 'osc2Enabled',
        value: 1,
        type: 'toggle',
        options: ['Off', 'On']
    },
    // {
    //     name: 'phaseOffset2',
    //     value: 0,
    //     min: 0,
    //     max: 1,
    //     step: 0.001,
    //     group: 'osc2',
    //     type: 'range'
    // },
    // {
    //     name: 'phaseMode2',
    //     value: 0,
    //     type: 'select',
    //     group: 'osc2',
    //     options: ['Off', 'Subtraction', 'Multiplication']
    // },

    // OSC 3
    {
        name: 'osc3Enabled',
        value: 0,
        type: 'toggle',
        options: ['Off', 'On']
    },
    {
        name: 'wave3',
        value: "/waves/fav/brown.ogg",
        type: 'wave',
        options: waves
    },
    {
        name: 'oct3',
        value: 0,
        type: "number",
        step: 1,
    },
    {
        name: 'semi3',
        type: "number",
        value: 0,
        min: -12,
        max: 12,
        step: 1,
    },
    {
        name: 'cent3',
        type: "number",
        value: 0,
        min: -100,
        max: 100,
        step: 1,
    },
    {
        name: 'wave3Mix',
        value: 0,
        min: 0,
        max: 1,
        step: 0.001,
        type: 'range'
    },

    // Add after the OSC 2 section and before the Mix section
    {
        name: 'wave3Level',
        value: 0,
        min: 0,
        max: 1,
        step: 0.001,
        type: 'range'
    },
    {
        name: 'wave3Decay',
        value: 0.1,
        min: 0,
        max: 1,
        step: 0.001,
        type: 'range'
    },

    // Mix section
    {
        name: 'mix',
        value: 0.5,
        min: 0,
        max: 1,
        step: 0.001,    
        type: 'range'
    },
    {
        name: 'fmAmount',
        value: 0,
        min: 0,
        max: 1,
        step: 0.01,
        type: 'range'
    },

    // Filter section
    {
        name: 'filterType',
        value: 0,
        type: 'select',
        options: ['Lowpass 24', 'Lowpass', 'Highpass', 'Bandpass', 'Notch']
    },
    {
        name: 'filterKeyTracking',
        value: 1.0,
        min: 0,
        max: 2,
        step: 0.01,
        type: 'range'
    },
    {
        name: 'cutoff',
        value: 1,
        min: 0,
        max: 1,
        step: 0.01,
        type: 'range'
    },
    {
        name: 'resonance',
        value: 0.7,
        min: 0.1,
        max: 10,
        step: 0.01,
        type: 'range'
    },

    // Filter Envelope
    {
        name: 'filterAttack',
        value: 0.1,
        min: 0,
        max: 2,
        step: 0.01,
        type: 'range'
    },
    {
        name: 'filterDecay',
        value: 0.1,
        min: 0,
        max: 2,
        step: 0.01,
        type: 'range'
    },
    {
        name: 'filterSustain',
        value: 0.7,
        min: 0,
        max: 1,
        step: 0.01,
        type: 'range'
    },
    {
        name: 'filterRelease',
        value: 0.1,
        min: 0,
        max: 2,
        step: 0.01,
        type: 'range'
    },
    {
        name: 'filterEnvAmount',
        value: 0.5,
        min: -1,
        max: 1,
        step: 0.01,
        type: 'range'
    },

    // Amp Envelope
    {
        name: 'ampAttack',
        value: 0.01,
        min: 0,
        max: 2,
        step: 0.01,
        type: 'range'
    },
    {
        name: 'ampDecay',
        value: 0.1,
        min: 0,
        max: 2,
        step: 0.01,
        type: 'range'
    },
    {
        name: 'ampSustain',
        value: 0.7,
        min: 0,
        max: 1,
        step: 0.01,
        type: 'range'
    },
    {
        name: 'ampRelease',
        value: 0.1,
        min: 0,
        max: 2,
        step: 0.01,
        type: 'range'
    },

    // LFO
    {
        name: "lfoWaveform",
        type: "select",
        value: 0,
        options: ["Triangle", "Sawtooth", "Square", "Sample & Hold", "Sine"]
    },
    {
        name: "lfoRetrigger",
        type: "toggle",
        value: 0,
        options: ["Off", "On"]
    },
    {
        name: "lfoRate",
        type: "range",
        value: 0.5,
        min: 0,
        max: 1,
        step: 0.01
    },
    {
        name: "lfoAmount",
        type: "range",
        value: 0,
        min: -1,
        max: 1,
        step: 0.01
    },
    {
        name: "lfoFadeIn",
        type: "range",
        value: 0,
        min: 0,
        max: 2,
        step: 0.01
    },
    {
        name: "lfoDestination",
        type: "select",
        value: 0,
        options: ["Oscillators", "Filter", "FM",  "Mix"]
    },

    // Master
    {
        name: 'portamento',
        value: 0,
        min: 0,
        max: 2,
        step: 0.01,
        type: 'range'
    },
    {
        name: 'autoPanWidth',
        value: 0,
        min: 0,
        max: 1,
        step: 0.01,
        type: 'range'
    },
    {
        name: 'autoPanRate',
        value: 0.5,
        min: 0,
        max: 10,
        step: 0.01,
        type: 'range'
    },
    {
        name: 'masterGain',
        value: 0.5,
        min: 0,
        max: 1,
        step: 0.01,
        type: 'range'
    },
    {
        name: 'polyphony',
        value: 4,
        min: 1,
        max: 8,
        step: 1,
        type: 'number'
    },
    // {
    //     name: 'noiseEnabled',
    //     value: 0,
    //     type: 'toggle',
    //     options: ['Off', 'On']
    // },
    {
        name: 'noiseDecay',
        value: 0.1,
        min: 0,
        max: 1,
        step: 0.001,
        type: 'range'
    },
    {
        name: 'noiseColor',
        value: 1,
        min: 0,
        max: 1,
        step: 0.001,
        type: 'range'
    },
    {
        name: 'noiseLevel',
        value: 0,
        min: 0,
        max: 1,
        step: 0.001,
        type: 'range'
    },

    // // Effects
    // {
    //     name: 'bitcrushAmount',
    //     value: 0,
    //     min: 0,
    //     max: 1,
    //     step: 0.01,
    //     type: 'range',
    //     group: 'effects'
    // },
    // {
    //     name: 'sampleReduction',
    //     value: 0,
    //     min: 0,
    //     max: 1,
    //     step: 0.01,
    //     type: 'range',
    //     group: 'effects'
    // }

    // Add these to the master section properties
    {
        name: 'distortion',
        value: 0,
        min: 0,
        max: 1,
        step: 0.01,
        type: 'range'
    },
    // {
    //     name: 'distortionCharacter',
    //     value: 0,
    //     min: 0,
    //     max: 1,
    //     step: 0.01,
    //     type: 'range'
    // },
];
