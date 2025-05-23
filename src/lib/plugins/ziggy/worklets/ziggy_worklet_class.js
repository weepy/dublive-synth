class ZiggyProcessor extends AudioWorkletProcessor {
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
                const {wave1, wave2, wave3, ...properties} = e.data.properties
                
                if(wave1) 
                    properties.wave1 = this.wavetableSlots.get(wave1)
                if(wave2)
                    properties.wave2 = this.wavetableSlots.get(wave2)
                if(wave3)
                    properties.wave3 = this.wavetableSlots.get(wave3)


                console.log("properties", properties)
                this.synth.setProperties(properties);
            }
            else if (e.data.type === 'debug') {
                this.debug = e.data.debug;
            }
            else if (e.data.type === 'loadwavetable') {
                const key = e.data.key;
                let slot = this.wavetableSlots.get(key);
                
                if (slot === undefined) {
                    slot = this.nextSlot++;
                    this.wavetableSlots.set(key, slot);
                }

                console.log("loadwavetable", key, this.wavetableSlots)
                
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
      

        if(this.debug) {
            for (let i = 0; i < 128; i++) {
                outputL[i] = this.outputHeap[i * 2];
                outputR[i] = this.outputHeap[i * 2 + 1];
            }
        }
        else {
  
            this.synth.processBuffer(this.outputPtr, 128);
        
            // Copy interleaved stereo data to separate channels
            for (let i = 0; i < 128; i++) {
                outputL[i] = this.outputHeap[i * 2];
                outputR[i] = this.outputHeap[i * 2 + 1];
            }
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

registerProcessor('ZiggyProcessor', ZiggyProcessor);

function generateTriangleWave(size = 2048) {
    const wave = new Float32Array(size);
    const period = size;
    const halfPeriod = period / 2;
    
    for (let i = 0; i < size; i++) {
        // Generate values from -1 to 1
        if (i < halfPeriod) {
            // Rising part: -1 to 1
            wave[i] = -1 + (2 * i / halfPeriod);
        } else {
            // Falling part: 1 to -1
            wave[i] = 1 - (2 * (i - halfPeriod) / halfPeriod);
        }
    }
    
    return wave;
}

// Generate a triangle wave table
const triangleWave = generateTriangleWave();