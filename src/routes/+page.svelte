<script>
    import { onMount } from 'svelte';

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
    let activeKeys = new Set();

    import { propertyDescriptors } from "$lib/ziggy/propertyDescriptors.js"


    let currentPreset = {}

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
    

    // Add this reactive statement after the currentPreset declaration
    $: if (synthNode && currentPreset) {
        synthNode.port.postMessage({
            type: 'properties',
            properties: currentPreset
        });

        
        localStorage.setItem('currentPreset', JSON.stringify(currentPreset))
    }

    const keyboardLayout = [
        { note: 60, key: 'Z', type: 'white' },
        { note: 61, key: 'S', type: 'black' },
        { note: 62, key: 'X', type: 'white' },
        { note: 63, key: 'D', type: 'black' },
        { note: 64, key: 'C', type: 'white' },
        { note: 65, key: 'V', type: 'white' },
        { note: 66, key: 'G', type: 'black' },
        { note: 67, key: 'B', type: 'white' },
        { note: 68, key: 'H', type: 'black' },
        { note: 69, key: 'N', type: 'white' },
        { note: 70, key: 'J', type: 'black' },
        { note: 71, key: 'M', type: 'white' },
    ];

    // Watch for parameter changes and update synth
    // $: if (synthNode && waveform !== undefined) {
    //     // sendWavetable(waveform);
    //     synthNode.port.postMessage({
    //         type: 'properties',
    //         properties: {
    //             wavetable: waveform,
    //             cutoff,
    //             resonance,
    //             attack,
    //             decay,
    //             sustain,
    //             release
    //         }
    //     });
    // }

    function handleNoteOn(note, keyElement) {
        synthNode.port.postMessage({ type: 'noteon', key: note, v: 1 });
        keyElement.style.background = keyElement.classList.contains('black-key') ? '#333' : '#ddd';
        activeKeys.add(note);
    }

    function handleNoteOff(note, keyElement) {
        synthNode.port.postMessage({ type: 'noteoff', key: note });
        keyElement.style.background = keyElement.classList.contains('black-key') ? 'black' : 'white';
        activeKeys.delete(note);
    }

    function handleKeyDown(e) {
        const key = keyboardLayout.find(k => k.key === e.key.toUpperCase());
        if (key && !e.repeat && !activeKeys.has(key.note)) {
            const keyElement = document.querySelector(`[data-note="${key.note}"]`);
            if (keyElement) {
                handleNoteOn(key.note, keyElement);
            }
        }
    }

    function handleKeyUp(e) {
        const key = keyboardLayout.find(k => k.key === e.key.toUpperCase());
        if (key) {
            const keyElement = document.querySelector(`[data-note="${key.note}"]`);
            if (keyElement) {
                handleNoteOff(key.note, keyElement);
            }
        }
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

            // Load all wavetables
            for (let i = 0; i < 3; i++) {
                sendWavetable(i);
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

    // Update the sendWavetable function
    function generateWavetable(type) {
        const size = 2048;
        const wavetable = new Float32Array(size);
        
        for (let i = 0; i < size; i++) {
            const phase = (i / size) * Math.PI * 2;
            
            switch (type) {
                case 0: // Sine
                    wavetable[i] = Math.sin(phase);
                    break;
                case 1: // Square
                    wavetable[i] = phase < Math.PI ? 1.0 : -1.0;
                    break;
                case 2: // Saw
                    wavetable[i] = 1.0 - (2.0 * (phase / (Math.PI * 2)));
                    break;
                default:
                    wavetable[i] = Math.sin(phase);
            }
        }
        return wavetable;
    }

    function sendWavetable(type) {
        if (!synthNode) return;
        
        const wavetable = generateWavetable(type);
        synthNode.port.postMessage({
            type: 'loadwavetable',
            key: type,  // Use the waveform type as the key
            table: wavetable
        }, [wavetable.buffer]);
    }
</script>

<div class="controls">
    <button class="play-button" on:click={togglePlay}>
        {isPlaying ? 'Stop' : 'Play'}
    </button>

    {#each [...new Set(propertyDescriptors.map(p => p.group))] as group}
        <div class="control-group">
            <h3>{group.toUpperCase()}</h3>
            <div class="group-controls">
                {#each propertyDescriptors.filter(p => p.group === group) as prop}
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
                        {/if}
                    </label>
                {/each}
            </div>
        </div>
    {/each}
</div>

<div class="keyboard">
    {#each keyboardLayout as { note, key, type }}
        <div
            class="key {type === 'black' ? 'black-key' : ''}"
            data-note={note}
            on:mousedown={(e) => handleNoteOn(note, e.currentTarget)}
            on:mouseup={(e) => handleNoteOff(note, e.currentTarget)}
            on:mouseleave={(e) => activeKeys.has(note) && handleNoteOff(note, e.currentTarget)}
        >
            {key}
        </div>
    {/each}
</div>

<svelte:window on:keydown={handleKeyDown} on:keyup={handleKeyUp} />

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
</style> 