class SynthProcessor extends AudioWorkletProcessor {
    constructor() {
        super();

        const mod = new Module();
        this.synth = new mod.PolySynth(44100, 16);
        this.mod = mod;  // Store the module instance
        
        // Pre-allocate the buffer memory once, using standard audio buffer size of 128
        this.outputPtr = this.mod._malloc(128 * 4);
        this.outputHeap = new Float32Array(this.mod.HEAPF32.buffer, this.outputPtr, 128);
        
        this.port.onmessage = (e) => {
            if (e.data.type === 'noteon') {
                this.synth.noteOn(e.data.key, e.data.v);
            }
            else if (e.data.type === 'noteoff') {
                this.synth.noteOff(e.data.key);
            }
     
            else if (e.data.type === 'properties') {
                this.synth.setProperties(e.data.properties);
            }
            else if (e.data.type === 'loadwavetable') {
                // Create a Float32Array view of the data
                // const wavetableArray = new Float32Array(e.data.table);
                // Pass the array directly to the WASM function
                this.synth.loadWavetable(e.data.key, new Float32Array(e.data.table));
            }
        };
    }
    
    process(inputs, outputs) {
        const output = outputs[0][0];
        this.synth.processBuffer(this.outputPtr, 128);
        output.set(this.outputHeap);
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