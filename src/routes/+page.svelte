<script lang="ts">
    import { onMount } from 'svelte';
    import Keyboard from '$lib/components/Keyboard.svelte';
    import SynthControls from '$lib/plugins/ziggy/ui/ZiggyControls.svelte';
    import PatchControls from '$lib/components/PatchControls.svelte';
    import Ziggy from '$lib/plugins/ziggy/Ziggy';
    import { waves } from '$lib/plugins/ziggy/waves.js';

    let synth;
    let audioContext: AudioContext;
    let isPlaying = false;
    let currentPreset: { name: string; [key: string]: any } = { name: 'Default' };
    let activeTab = 'oscillators';
    let patches: { [key: string]: any } = {};

    onMount(async () => {
        // Load saved patches from localStorage
        try {
            const savedPatches = localStorage.getItem('patches');
            if (savedPatches) {
                patches = JSON.parse(savedPatches);
            } else {
                patches = {
                    'Default': { ...currentPreset }
                };
            }
        } catch (error) {
            console.error('Error loading patches:', error);
            patches = {
                'Default': { ...currentPreset }
            };
        }

        // Initialize properties from descriptors
        // propertyDescriptors.forEach(prop => {
        //     currentPreset[prop.name] = prop.value;
        // });

        try {
            const savedPresetStr = localStorage.currentPreset;
            if (savedPresetStr) {
                const o = JSON.parse(savedPresetStr);
                currentPreset = { ...currentPreset, ...o };
            }
        } catch (error) {
            console.error('Error loading preset:', error);
        }

        if (!currentPreset.name) {
            currentPreset.name = 'Default';
        }
    });

    async function init() {
        audioContext = new AudioContext();
        synth = new Ziggy({}, { audioContext });
        await synth.buildGraph();

        const preset = { ...Ziggy.defaultProperties(), ...currentPreset };

        synth.handleUpdate(preset);

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

    function handleNoteOn(note: number, keyElement: HTMLElement) {
        synth.noteon(note);
        keyElement.style.background = keyElement.classList.contains('black-key') ? '#333' : '#ddd';
    }

    function handleNoteOff(note: number, keyElement: HTMLElement) {
        synth.noteoff(note);
        keyElement.style.background = keyElement.classList.contains('black-key') ? 'black' : 'white';
    }

    // // Update the reactive statement to use the new synth class
    $: if (synth?.ready && currentPreset) {
        synth.handleUpdate(currentPreset);
        localStorage.setItem('currentPreset', JSON.stringify(currentPreset));
    }


</script>

    {#if !isPlaying}
        <button class="play-button" on:click={togglePlay}>Play</button>
    {:else}
    <Keyboard 
        onNoteOn={handleNoteOn}
        onNoteOff={handleNoteOff}
    />

    <PatchControls
        bind:currentPreset
        bind:patches
    />
    
    <SynthControls
        bind:currentPreset
        {waves}
    />

    {/if}

<style>
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