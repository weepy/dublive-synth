<script>
    import { onMount } from 'svelte';
    import Keyboard from '$lib/components/Keyboard.svelte';

    import ziggyWaves from '$lib/ziggy/waves.json';

    let synth;
    let audioContext;
    let isPlaying = false;

    // Reactive declarations for synth parameters
    let waveform = 2; // Default to saw wave
    // let cutoff = 1000;
    // let resonance = 0.7;
    // let attack = 0.5;
    // let decay = 0.1;
    // let sustain = 0.7;
    // let release = 0.9;
    let synthNode;

    // Track active keys
    // let activeKeys = new Set();

    import { propertyDescriptors } from "$lib/ziggy/propertyDescriptors.js"


    let currentPreset = {}

    // Change this line to use tab instead of group for tab organization
    let activeTab = 'oscillators'; 

    onMount(() => {
        

        propertyDescriptors.forEach(prop => {
            currentPreset[prop.name] = prop.value;
        });

        try {
            const savedPresetStr = localStorage.currentPreset
            if (savedPresetStr) {
                const o = JSON.parse(savedPresetStr);
            for(let i in o) {
                    currentPreset[i] = o[i];
                }
            }
        } catch (error) {
            console.error('Error loading preset:', error);
        }

    });
    

    // Update the reactive statement to handle wavetable changes
    $: if (synthNode && currentPreset) {
        // Create an async function to handle all updates
        (async () => {

            console.log(currentPreset.wave1);
            // Load wavetables first if needed
            if (currentPreset.wave1) {
                await sendWavetable(currentPreset.wave1);
            }

            console.log(currentPreset.wave2);
            if (currentPreset.wave2) {
                await sendWavetable(currentPreset.wave2);
            }

            // Send other properties after wavetables are loaded
            synthNode.port.postMessage({
                type: 'properties',
                properties: currentPreset
            });
            
            localStorage.setItem('currentPreset', JSON.stringify(currentPreset));
        })();
    }

    // Add a cache for loaded wavetables
    let wavetableCache = new Map();

    import { decodeAudioDataAny } from "$lib/utils.js"

    // Function to load and decode audio file
    async function loadWavetableFromUrl(url) {
        if (wavetableCache.has(url)) {
            return wavetableCache.get(url);
        }

        const response = await fetch(url);
        const arrayBuffer = await response.arrayBuffer();
        const audioBuffer = await decodeAudioDataAny("audio/ogg", arrayBuffer, audioContext);
        
        // Convert audio buffer to wavetable (using first channel)
        
        const data = audioBuffer.getChannelData(0);
        
        // const wavetable = new Float32Array(2048);

        // // Resample to 2048 points using linear interpolation
        // for (let i = 0; i < 2048; i++) {
        //     const position = (i / 2048) * (data.length - 1); // Adjust position calculation
        //     const index1 = Math.floor(position);
        //     const index2 = Math.min(index1 + 1, data.length - 1);
        //     const fraction = position - index1;
            
        //     wavetable[i] = (1 - fraction) * data[index1] + fraction * data[index2]; // Clearer lerp formula
        // }
        
                
        const wavetable = new Float32Array(data.length);
        // Resample to 2048 points
        for (let i = 0; i < data.length; i++) {
            // const index = Math.floor((i / 2048) * data.length);
            wavetable[i] = data[i];
        }

        wavetableCache.set(url, wavetable);
        return wavetable;
    }

    // function intFromStr(str) {
    //     let hash = 0;
    //     for (let i = 0; i < str.length; i++) {
    //         const char = str.charCodeAt(i);
    //         hash = (hash << 5) - hash + char;
    //         hash |= 0; // Convert to 32bit integer
    //     }
    //     return ziggyWaves.indexOf(str);
    // }
    // Update the sendWavetable function to return a promise
    async function sendWavetable(url) {
        if (!synthNode) return;
        
        // Ensure url is valid
        if (!ziggyWaves.includes(url)) {
            url = ziggyWaves[0];
        }

        // Skip if we've already sent this wavetable
        if (wavetableCache.has(url)) return;

        let wavetable = await loadWavetableFromUrl(url);
        
        console.log(wavetable);

        synthNode.port.postMessage({
            type: 'loadwavetable',
            key: url,
            table: wavetable
        }, [wavetable.buffer]);
    }

    function handleNoteOn(note, keyElement) {
        synthNode.port.postMessage({ type: 'noteon', key: note, v: 1 });
        keyElement.style.background = keyElement.classList.contains('black-key') ? '#333' : '#ddd';
        // activeKeys.add(note);
    }

    function handleNoteOff(note, keyElement) {
        synthNode.port.postMessage({ type: 'noteoff', key: note });
        keyElement.style.background = keyElement.classList.contains('black-key') ? 'black' : 'white';
        // activeKeys.delete(note);
    }

    async function init() {
        try {
            audioContext = new AudioContext();
            await audioContext.audioWorklet.addModule("/worklets/synth_worklet.js");
            synthNode = new AudioWorkletNode(audioContext, 'synth-processor',{
                numberOfOutputs: 1,
                outputChannelCount: [2]  // Specify 2 channels for the output
            });
            synthNode.connect(audioContext.destination);

            if (audioContext.state === 'suspended') {
                await audioContext.resume();
            }
            

            // synthNode.port.postMessage({
            //     type: 'debug',
            //     debug: true
            // })
            
            // Send initial parameters
            synthNode.port.postMessage({
                type: 'properties',
                properties: currentPreset
            });
        } catch (error) {
            console.error('Init error:', error);
        }
    }

    async function togglePlay() {
        if (!audioContext) {
            await init();
        }
        isPlaying = !isPlaying;
        if (isPlaying) {
            await audioContext.resume();
        } else {
            await audioContext.suspend();
        }
    }

  
    function next(prop, dir = 1) {
        const currentUrl = currentPreset[prop];
        const currentIndex = ziggyWaves.indexOf(currentUrl);
        const newIndex = (currentIndex + dir + ziggyWaves.length) % ziggyWaves.length;
        currentPreset[prop] = ziggyWaves[newIndex] || ziggyWaves[0];

        console.log(currentPreset[prop]);
    }
</script>

<div class="controls">
    
    
    {#if !isPlaying}
    <button class="play-button" on:click={togglePlay}>
        {isPlaying ? 'Stop' : 'Play'}
    </button>
    {:else}

    <Keyboard 
        onNoteOn={handleNoteOn}
        onNoteOff={handleNoteOff}
        
    />

        <div class="tabs">
            <button class="tab-button" class:active={activeTab === 'oscillators'} on:click={() => activeTab = 'oscillators'}>
                OSCILLATORS
            </button>
            <button class="tab-button" class:active={activeTab === 'filter'} on:click={() => activeTab = 'filter'}>
                FILTER
            </button>
            <button class="tab-button" class:active={activeTab === 'envelope'} on:click={() => activeTab = 'envelope'}>
                ENVELOPE
            </button>
            <button class="tab-button" class:active={activeTab === 'modulation'} on:click={() => activeTab = 'modulation'}>
                LFO
            </button>
            <button class="tab-button" class:active={activeTab === 'noise'} on:click={() => activeTab = 'noise'}>
                NOISE
            </button>
            <button class="tab-button" class:active={activeTab === 'master'} on:click={() => activeTab = 'master'}>
                MASTER
            </button>
        </div>

        {#if activeTab === 'oscillators'}
            <div class="control-group">
                <div class="group-controls">
                    <!-- <h3>Oscillator 1</h3> -->
                    <label>
                        Wave 1:
                        <div class="wave-selector">
                            <button class="wave-btn" on:click={() => next('wave1', -1)}>←</button>
                            <select bind:value={currentPreset.wave1}>
                                {#each ziggyWaves as url}
                                    <option value={url}>
                                        {url.split('/').pop().replace(/\..*$/, '').replace(/[-_]/g, ' ')}
                                    </option>
                                {/each}
                            </select>
                            <button class="wave-btn" on:click={() => next('wave1', 1)}>→</button>
                        </div>
                    </label>
                    <label>
                        Pitch 1:
                        <div class="pitch-control">
                            <input type="number" bind:value={currentPreset.oct1} min={-4} max={4} step={1} title="Octave">
                            <input type="number" bind:value={currentPreset.semi1} min={-12} max={12} step={1} title="Semitones">
                            <input type="number" bind:value={currentPreset.cent1} min={-100} max={100} step={1} title="Cents">
                        </div>
                    </label>

                    <h3></h3>
                    <label>
                        Enable:
                        <select bind:value={currentPreset.osc2Enabled}>
                            <option value={0}>Off</option>
                            <option value={1}>On</option>
                        </select>
                    </label>
                    <label>
                        Wave 2:
                        <div class="wave-selector">
                            <button class="wave-btn" on:click={() => next('wave2', -1)}>←</button>
                            <select bind:value={currentPreset.wave2}>
                                {#each ziggyWaves as url}
                                    <option value={url}>
                                        {url.split('/').pop().replace(/\..*$/, '').replace(/[-_]/g, ' ')}
                                    </option>
                                {/each}
                            </select>
                            <button class="wave-btn" on:click={() => next('wave2', 1)}>→</button>
                        </div>
                    </label>
                    <label>
                        Pitch 2:
                        <div class="pitch-control">
                            <input type="number" bind:value={currentPreset.oct2} min={-4} max={4} step={1} title="Octave">
                            <input type="number" bind:value={currentPreset.semi2} min={-12} max={12} step={1} title="Semitones">
                            <input type="number" bind:value={currentPreset.cent2} min={-100} max={100} step={1} title="Cents">
                        </div>
                    </label>

                    <h3></h3>
                    <label>
                        Mix:
                        <input type="range" bind:value={currentPreset.mix} min={0} max={1} step={0.001}>
                        <span class="value-display">{(currentPreset.mix ?? 0).toFixed(2)}</span>
                    </label>
                    <label>
                        FM Amount:
                        <input type="range" bind:value={currentPreset.fmAmount} min={0} max={1} step={0.01}>
                        <span class="value-display">{(currentPreset.fmAmount ?? 0).toFixed(2)}</span>
                    </label>
                </div>
            </div>
        {/if}

        
        {#if activeTab === 'filter'}
            <div class="control-group">
                <div class="group-controls">
                    
                    <label>
                        Type:
                        <select bind:value={currentPreset.filterType}>
                            <option value={0}>Lowpass 24</option>
                            <option value={1}>Lowpass</option>
                            <option value={2}>Highpass</option>
                            <option value={3}>Bandpass</option>
                            <option value={4}>Notch</option>
                        </select>
                    </label>
                    <label>
                        Cutoff:
                        <input type="range" bind:value={currentPreset.cutoff} min={0} max={1} step={0.01}>
                        <span class="value-display">{(currentPreset.cutoff ?? 0).toFixed(2)}</span>
                    </label>
                    <label>
                        Resonance:
                        <input type="range" bind:value={currentPreset.resonance} min={0.1} max={10} step={0.01}>
                        <span class="value-display">{(currentPreset.resonance ?? 0).toFixed(2)}</span>
                    </label>

                    <h3></h3>
                    <label>
                        Attack:
                        <input type="range" bind:value={currentPreset.filterAttack} min={0} max={2} step={0.01}>
                        <span class="value-display">{(currentPreset.filterAttack ?? 0).toFixed(2)}</span>
                    </label>
                    <label>
                        Decay:
                        <input type="range" bind:value={currentPreset.filterDecay} min={0} max={2} step={0.01}>
                        <span class="value-display">{(currentPreset.filterDecay ?? 0).toFixed(2)}</span>
                    </label>
                    <label>
                        Sustain:
                        <input type="range" bind:value={currentPreset.filterSustain} min={0} max={1} step={0.01}>
                        <span class="value-display">{(currentPreset.filterSustain ?? 0).toFixed(2)}</span>
                    </label>
                    <label>
                        Release:
                        <input type="range" bind:value={currentPreset.filterRelease} min={0} max={2} step={0.01}>
                        <span class="value-display">{(currentPreset.filterRelease ?? 0).toFixed(2)}</span>
                    </label>
                    <label>
                        Envelope Amount:
                        <input type="range" bind:value={currentPreset.filterEnvAmount} min={-1} max={1} step={0.01}>
                        <span class="value-display">{(currentPreset.filterEnvAmount ?? 0).toFixed(2)}</span>
                    </label>
                </div>
            </div>
        {/if}

        {#if activeTab === 'envelope'}
            <div class="control-group">
                <div class="group-controls">
                    <!-- <h3>Amplitude Envelope</h3> -->
                    <label>
                        Attack:
                        <input type="range" bind:value={currentPreset.ampAttack} min={0} max={2} step={0.01}>
                        <span class="value-display">{(currentPreset.ampAttack ?? 0).toFixed(2)}</span>
                    </label>
                    <label>
                        Decay:
                        <input type="range" bind:value={currentPreset.ampDecay} min={0} max={2} step={0.01}>
                        <span class="value-display">{(currentPreset.ampDecay ?? 0).toFixed(2)}</span>
                    </label>
                    <label>
                        Sustain:
                        <input type="range" bind:value={currentPreset.ampSustain} min={0} max={1} step={0.01}>
                        <span class="value-display">{(currentPreset.ampSustain ?? 0).toFixed(2)}</span>
                    </label>
                    <label>
                        Release:
                        <input type="range" bind:value={currentPreset.ampRelease} min={0} max={2} step={0.01}>
                        <span class="value-display">{(currentPreset.ampRelease ?? 0).toFixed(2)}</span>
                    </label>
                </div>
            </div>
        {/if}

        {#if activeTab === 'modulation'}
            <div class="control-group">
                <div class="group-controls">
                    <!-- <h3>LFO</h3> -->
                    <label>
                        Waveform:
                        <select bind:value={currentPreset.lfoWaveform}>
                            <option value={0}>Triangle</option>
                            <option value={1}>Sawtooth</option>
                            <option value={2}>Square</option>
                            <option value={3}>Sample & Hold</option>
                            <option value={4}>Sine</option>
                        </select>
                    </label>
                    <label>
                        Retrigger:
                        <select bind:value={currentPreset.lfoRetrigger}>
                            <option value={0}>Off</option>
                            <option value={1}>On</option>
                        </select>
                    </label>
                    <label>
                        Rate:
                        <input type="range" bind:value={currentPreset.lfoRate} min={0} max={1} step={0.01}>
                        <span class="value-display">{(currentPreset.lfoRate ?? 0).toFixed(2)}</span>
                    </label>
                    <label>
                        Amount:
                        <input type="range" bind:value={currentPreset.lfoAmount} min={0} max={1} step={0.01}>
                        <span class="value-display">{(currentPreset.lfoAmount ?? 0).toFixed(2)}</span>
                    </label>
                    <label>
                        Fade In:
                        <input type="range" bind:value={currentPreset.lfoFadeIn} min={0} max={2} step={0.01}>
                        <span class="value-display">{(currentPreset.lfoFadeIn ?? 0).toFixed(2)}</span>
                    </label>
                    <label>
                        Destination:
                        <select bind:value={currentPreset.lfoDestination}>
                            <option value={0}>Oscillators</option>
                            <option value={1}>Filter</option>
                            <option value={2}>FM</option>
                            <option value={3}>Mix</option>
                        </select>
                    </label>
                </div>
            </div>
        {/if}

        {#if activeTab === 'noise'}
            <div class="control-group">
                <div class="group-controls">
                    <label>
                        Level:
                        <input type="range" bind:value={currentPreset.noiseLevel} min={0} max={1} step={0.001}>
                        <span class="value-display">{(currentPreset.noiseLevel ?? 0).toFixed(3)}</span>
                    </label>
                    <!-- <h3>Noise</h3> -->
                    <!-- <label>
                        Enable:
                        <select bind:value={currentPreset.noiseEnabled}>
                            <option value={0}>Off</option>
                            <option value={1}>On</option>
                        </select>
                    </label> -->
                    <label>
                        Decay:
                        <input type="range" bind:value={currentPreset.noiseDecay} min={0} max={1} step={0.001}>
                        <span class="value-display">{(currentPreset.noiseDecay ?? 0).toFixed(3)}</span>
                    </label>
                    <label>
                        Color:
                        <input type="range" bind:value={currentPreset.noiseColor} min={0} max={1} step={0.001}>
                        <span class="value-display">{(currentPreset.noiseColor ?? 0).toFixed(3)}</span>
                    </label>
                    
                </div>
            </div>
        {/if}

        {#if activeTab === 'master'}
            <div class="control-group">
                <div class="group-controls">
                    <label>
                        Portamento:
                        <input type="range" bind:value={currentPreset.portamento} min={0} max={2} step={0.01}>
                        <span class="value-display">{(currentPreset.portamento ?? 0).toFixed(2)}</span>
                    </label>
                    <label>
                        Panning:
                        <input type="range" bind:value={currentPreset.autoPanWidth} min={0} max={1} step={0.01}>
                        <span class="value-display">{(currentPreset.autoPanWidth ?? 0).toFixed(2)}</span>
                    </label>
                    <label>
                        Pan Rate:
                        <input type="range" bind:value={currentPreset.autoPanRate} min={0} max={10} step={0.01}>
                        <span class="value-display">{(currentPreset.autoPanRate ?? 0).toFixed(2)}</span>
                    </label>
                    <label>
                        Gain:
                        <input type="range" bind:value={currentPreset.masterGain} min={0} max={1} step={0.01}>
                        <span class="value-display">{(currentPreset.masterGain ?? 0).toFixed(2)}</span>
                    </label>
                    <label>
                        Polyphony:
                        <input type="number" bind:value={currentPreset.polyphony} min={1} max={8} step={1}>
                    </label>
                </div>
            </div>
        {/if}
    {/if}
</div>



<style>
    .keyboard {
        display: flex;
        justify-content: center;
        margin: 20px;
        position: relative;
    }
    .key {
        width: 40px;
        height: 120px;
        border: 1px solid black;
        background: white;
        margin: 0 2px;
        cursor: pointer;
        display: flex;
        align-items: flex-end;
        justify-content: center;
        padding-bottom: 10px;
    }
    .black-key {
        width: 30px;
        height: 80px;
        background: black;
        position: relative;
        margin-left: -15px;
        margin-right: -15px;
        z-index: 1;
        color: white;
    }
    .controls {
        display: flex;
        flex-wrap: wrap;
        gap: 20px;
        padding: 20px;
        justify-content: center;
    }

    .control-group {
        background: #f5f5f5;
        padding: 15px;
        border-radius: 8px;
        min-width: 250px;
    }

    .control-group h3 {
        margin: 0 0 10px 0;
        color: #333;
        font-size: 1.1em;
        border-bottom: 2px solid #ddd;
        padding-bottom: 5px;
    }

    .group-controls {
        display: flex;
        flex-direction: column;
        gap: 8px;
    }

    label {
        display: flex;
        align-items: center;
        justify-content: space-between;
        gap: 10px;
    }

    input[type="range"] {
        flex: 1;
    }

    .value-display {
        min-width: 45px;
        text-align: right;
        font-family: monospace;
    }

    select, input[type="number"] {
        width: 80px;
    }

    .play-button {
        padding: 10px 20px;
        font-size: 16px;
        margin-bottom: 20px;
        background-color: #4CAF50;
        color: white;
        border: none;
        border-radius: 4px;
        cursor: pointer;
    }

    .play-button:hover {
        background-color: #45a049;
    }

    .wave-selector {
        display: flex;
        align-items: center;
        gap: 5px;
    }

    .wave-btn {
        padding: 2px 8px;
        cursor: pointer;
        background: #eee;
        border: 1px solid #ccc;
        border-radius: 4px;
    }

    .wave-btn:hover {
        background: #ddd;
    }

    .wave-selector select {
        flex: 1;
    }

    .tabs {
        display: flex;
        gap: 4px;
        margin-bottom: 20px;
        width: 100%;
        justify-content: center;
    }

    .tab-button {
        padding: 8px 16px;
        border: none;
        background: #f0f0f0;
        cursor: pointer;
        border-radius: 4px;
        transition: background-color 0.2s;
    }

    .tab-button:hover {
        background: #e0e0e0;
    }

    .tab-button.active {
        background: #4CAF50;
        color: white;
    }

    .control-group {
        width: 100%;
        max-width: 600px;
        margin: 0 auto;
    }

    .pitch-control {
        display: flex;
        gap: 4px;
    }

    .pitch-control input[type="number"] {
        width: 60px;
        text-align: center;
    }
</style> 