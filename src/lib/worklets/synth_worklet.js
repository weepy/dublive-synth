class SynthProcessor extends AudioWorkletProcessor {
    constructor() {
        super();

        const mod = new Module();
        this.synth = new mod.PolySynth(44100, 16);
        this.mod = mod;  // Store the module instance
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
        const outputPtr = this.mod._malloc(output.length * 4);
        const outputHeap = new Float32Array(this.mod.HEAPF32.buffer, outputPtr, output.length);
        
        this.synth.processBuffer(outputPtr, output.length);
        
        output.set(outputHeap);
        
        this.mod._free(outputPtr);
        
        return true;
    }
}

registerProcessor('synth-processor', SynthProcessor);