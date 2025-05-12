import { decodeAudioDataAny } from "$lib/utils/utils.js";
import ZiggyProcessor from "./worklets/ZiggyProcessor.js?url"

export default class Ziggy {
    constructor(properties, { audioContext }) {
        this.audioContext = audioContext;
        this.audioCache = new Map();
        this.properties = {};
        this.synthNode = null;
        // this.ready = new Promise((resolve, reject) => {
        //     this._resolveReady = resolve;
        //     this._rejectReady = reject;
        // });
    }

    static defaultProperties() {

        return {
            "wave1":"/plugins/ziggy/waves/add_waves/wave/color3.L.ogg",
            "oct1":0,
            "semi1":0,
            "cent1":-3,
            "wave2":"/plugins/ziggy/waves/add_waves/wave/color3.L.ogg",
            "oct2":0,
            "semi2":0,
            "cent2":7,
            "osc2Enabled":1,
            "mix":0.5,
            "fmAmount":0,
            "filterType":0,
            "cutoff":0.11,
            "resonance":0.7,
            "filterAttack":0.1,
            "filterDecay":0.1,
            "filterSustain":0.7,
            "filterRelease":0.1,
            "filterEnvAmount":0.5,
            "ampAttack":0.01,
            "ampDecay":0.1,
            "ampSustain":0.7,
            "ampRelease":0.1,
            "lfoWaveform":0,
            "lfoRetrigger":0,
            "lfoRate":0.5,
            "lfoAmount":0,
            "lfoFadeIn":0,
            "lfoDestination":0,
            "portamento":0,
            "autoPanWidth":0,
            "autoPanRate":0.5,
            "masterGain":0.5,
            "polyphony":4,
            "noiseDecay":0.1,
            "noiseColor":1,
            "noiseLevel":0
        }


    }

    async buildGraph() {
        // try {
            await this.audioContext.audioWorklet.addModule(ZiggyProcessor);
            this.synthNode = new AudioWorkletNode(this.audioContext, 'ZiggyProcessor', {
                numberOfOutputs: 1,
                outputChannelCount: [2]
            });
            this.synthNode.connect(this.audioContext.destination);
            this.ready = true;

            // this._resolveReady();
        // } catch (error) {
        //     this._rejectReady(error);
        //     throw error;
        // }
    }

    async handleUpdate(properties) {
        // Update local properties

        const changedProperties = {};

        for(let key in properties) {
            if(this.properties[key] !== properties[key]) {
                changedProperties[key] = properties[key];
                this.properties[key] = properties[key];
            }
        }

        if(Object.keys(changedProperties).length > 0) {
            // Handle wavetable updates first
            if (changedProperties.wave1) await this.sendWavetable(properties.wave1);
            if (changedProperties.wave2) await this.sendWavetable(properties.wave2);
            if (changedProperties.wave3) await this.sendWavetable(properties.wave3);

            this.synthNode.port.postMessage({
                type: 'properties',
                properties: changedProperties
            });
        }


    }

    // async sendProperties() {
    //     this.synthNode.port.postMessage({
    //         type: 'properties',
    //         properties: this.properties
    //     });
    // }

    async preloadAudio(url) {
        if (this.audioCache.has(url)) {
            return this.audioCache.get(url);
        }

        const response = await fetch(url);
        const arrayBuffer = await response.arrayBuffer();
        const audioBuffer = await decodeAudioDataAny("audio/ogg", arrayBuffer, this.audioContext);
        
        const data = audioBuffer.getChannelData(0);
        const WAVETABLE_SIZE = 1348;
        
        let wavetable;
        
        if (data.length >= 10000) {
            wavetable = data;
        } else {
            const multiplier = Math.max(1, Math.round(data.length / WAVETABLE_SIZE));
            const finalSize = WAVETABLE_SIZE * multiplier;
            
            wavetable = new Float32Array(finalSize);
            
            for (let i = 0; i < finalSize; i++) {
                const position = (i / finalSize) * data.length;
                const index1 = Math.floor(position) % data.length;
                const index2 = (index1 + 1) % data.length;
                const fraction = position - Math.floor(position);
                
                wavetable[i] = (1 - fraction) * data[index1] + fraction * data[index2];
            }
        }

        this.audioCache.set(url, wavetable);
        return wavetable;
    }

    async sendWavetable(url) {
        if (!this.synthNode) return;

        const wavetable = await this.preloadAudio(url);
        
        const wavetableCopy = new Float32Array(wavetable);
        
        
        this.synthNode.port.postMessage({
            type: 'loadwavetable',
            key: url,
            table: wavetableCopy
        }, [wavetableCopy.buffer]);
    }

    noteon(note, velocity = 1) {
        this.synthNode.port.postMessage({ type: 'noteon', key: note, v: velocity });
    }

    noteoff(note) {
        this.synthNode.port.postMessage({ type: 'noteoff', key: note });
    }

    abortAllNotes() {
        this.synthNode.port.postMessage({ type: 'abortall' });
    }
} 