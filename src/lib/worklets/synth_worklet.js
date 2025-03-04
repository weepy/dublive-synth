class SynthProcessor extends AudioWorkletProcessor {
    constructor() {
        super();

        const mod = new Module();
        this.synth = new mod.PolySynth(44100, 16);
        this.mod = mod;  // Store the module instance
        
        // Pre-allocate the buffer memory once, using standard audio buffer size of 128
        this.outputPtr = this.mod._malloc(128 * 4 * 2);
        this.outputHeap = new Float32Array(this.mod.HEAPF32.buffer, this.outputPtr, 128 * 2);
        
        // Keep track of wavetable URL to slot mappings
        this.wavetableSlots = new Map();
        this.nextSlot = 0;

        this.port.onmessage = (e) => {
            if (e.data.type === 'noteon') {
                this.synth.noteOn(e.data.key, e.data.v);
            }
            else if (e.data.type === 'noteoff') {
                this.synth.noteOff(e.data.key);
            }
            else if (e.data.type === 'properties') {
                // Replace wavetable URLs with their corresponding slot numbers
                const {wave1, wave2, ...properties} = e.data.properties
                
                if(wave1) 
                    properties.wave1 = this.wavetableSlots.get(wave1)
                if(wave2)
                    properties.wave2 = this.wavetableSlots.get(wave2)
                
                this.synth.setProperties(properties);
            }
            else if (e.data.type === 'loadwavetable') {
                const key = e.data.key;
                let slot = this.wavetableSlots.get(key);
                
                if (slot === undefined) {
                    slot = this.nextSlot++;
                    this.wavetableSlots.set(key, slot);
                }
                
                this.synth.loadWavetable(slot, new Float32Array(e.data.table));
            }
        };
    }
    
    process(inputs, outputs) {
        const output = outputs[0];  // Get first output
        
        // Check if we have at least 2 channels
        if (output.length < 2) {
            console.error('Output needs at least 2 channels');
            return true;
        }
        
        const outputL = output[0];
        const outputR = output[1];
        
        this.synth.processBuffer(this.outputPtr, 128);
        
        // Copy interleaved stereo data to separate channels
        for (let i = 0; i < 128; i++) {
            outputL[i] = this.outputHeap[i * 2];
            outputR[i] = this.outputHeap[i * 2 + 1];
        }
        return true;
    }
    
    disconnect() {
        if (this.outputPtr) {
            this.mod._free(this.outputPtr);
            this.outputPtr = null;
            this.outputHeap = null;
        }
    }
}

registerProcessor('synth-processor', SynthProcessor);