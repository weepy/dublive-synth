class SynthProcessor extends AudioWorkletProcessor {
    constructor() {
        super();
        this.phase = 0;
        this.frequency = 440; // 440 Hz (A4 note)
    }
    
    process(inputs, outputs) {
        const output = outputs[0];
        
        if (output.length < 2) {
            console.error('Output needs at least 2 channels');
            return true;
        }
        
        const outputL = output[0];
        const outputR = output[1];
        
        // Generate a sine wave
        for (let i = 0; i < 128; i++) {
            // Calculate the sine wave value
            const value = 0.5 * Math.sin(2 * Math.PI * this.phase);
            
            // Update both channels with the same value
            outputL[i] = value;
            outputR[i] = value;
            
            // Increment phase
            this.phase += this.frequency / sampleRate;
            if (this.phase > 1) this.phase -= 1;
        }

        return true;
    }
    
    disconnect() {
        // Cleanup if needed
    }
}

registerProcessor('synth-processor', SynthProcessor);

