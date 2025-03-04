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
    let activeTab = propertyDescriptors[0]?.tab || '';

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
        // Send wavetable if it's a wavetable property
        
        if (currentPreset.wave1) {
            // const url = ziggyWaves[currentPreset[prop.name]];
            sendWavetable(currentPreset.wave1);
            // Update the preset to use the URL instead of the index
            // currentPreset[prop.name] = url;
        }
        if(currentPreset.wave2) {
            sendWavetable(currentPreset.wave2);
        }

        // Send other properties
        synthNode.port.postMessage({
            type: 'properties',
            properties: currentPreset
        });
        
        localStorage.setItem('currentPreset', JSON.stringify(currentPreset));
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
    // Update the sendWavetable function
    async function sendWavetable(url) {
        if (!synthNode) return;
        
        // Ensure url is valid
        if (!ziggyWaves.includes(url)) {
            url = ziggyWaves[0];
        }

        // Skip if we've already sent this wavetable
        if (wavetableCache.has(url)) return;

        let wavetable = await loadWavetableFromUrl(url);
        
        synthNode.port.postMessage({
            type: 'loadwavetable',
            key: url,  // Send the URL as the key instead of index
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
            synthNode = new AudioWorkletNode(audioContext, 'synth-processor');
            synthNode.connect(audioContext.destination);

            if (audioContext.state === 'suspended') {
                await audioContext.resume();
            }
            
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

    onMount(() => {
        init();
    });

    function next(prop, dir = 1) {
        const currentUrl = currentPreset[prop];
        const currentIndex = ziggyWaves.indexOf(currentUrl);
        const newIndex = (currentIndex + dir + ziggyWaves.length) % ziggyWaves.length;
        currentPreset[prop] = ziggyWaves[newIndex] || ziggyWaves[0];

        console.log(currentPreset[prop]);
    }
</script>

<div class="controls">
    <button class="play-button" on:click={togglePlay}>
        {isPlaying ? 'Stop' : 'Play'}
    </button>
    
    <div class="tabs">
        {#each [...new Set(propertyDescriptors.map(p => p.tab))] as tab}
            <button 
                class="tab-button" 
                class:active={activeTab === tab}
                on:click={() => activeTab = tab}
            >
                {tab?.toUpperCase()}
            </button>
        {/each}
    </div>

    {#each [...new Set(propertyDescriptors.map(p => p.tab))] as tab}
        {#if activeTab === tab}
            <div class="control-group">
                <div class="group-controls">
                    {#each propertyDescriptors.filter(p => p.tab === tab) as prop}
                        <label>
                            {prop.name}:
                            {#if prop.type == 'select'}
                                <select bind:value={currentPreset[prop.name]}>
                                    {#each prop.options as option, index}
                                        <option value={index}>{option}</option>
                                    {/each}
                                </select>
                            {:else if prop.type == "number"}
                                <input type="number" 
                                    bind:value={currentPreset[prop.name]} 
                                    min={prop.min} 
                                    max={prop.max} 
                                    step={prop.step}>
                            {:else if prop.type == "range"}
                                <input type="range" 
                                    bind:value={currentPreset[prop.name]} 
                                    min={prop.min} 
                                    max={prop.max} 
                                    step={prop.step}>
                                <span class="value-display">
                                    {(currentPreset[prop.name] ?? 0).toFixed(2)}
                                </span>
                            {:else if prop.type == "toggle"}
                                <select bind:value={currentPreset[prop.name]}>
                                    {#each prop.options as option, index}
                                        <option value={index}>{option}</option>
                                    {/each}
                                </select>
                            {:else if prop.type == 'wave'}
                                <div class="wave-selector">
                                    <button class="wave-btn" on:click={() => next(prop.name, -1)}>←</button>
                                    
                                    <select bind:value={currentPreset[prop.name]}>
                                        {#each ziggyWaves as url}
                                            <option value={url}>
                                                {url.split('/').pop().replace(/\..*$/, '').replace(/[-_]/g, ' ')}
                                            </option>
                                        {/each}
                                    </select>

                                    <button class="wave-btn" on:click={() => next(prop.name, 1)}>→</button>
                                </div>
                            {/if}
                        </label>
                    {/each}
                </div>
            </div>
        {/if}
    {/each}
</div>

<Keyboard 
    onNoteOn={handleNoteOn}
    onNoteOff={handleNoteOff}
    
/>

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
</style> 