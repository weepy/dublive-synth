<script>
    import OscillatorPanel from './panels/OscillatorPanel.svelte';
    import FilterPanel from './panels/FilterPanel.svelte';
    import EnvelopePanel from './panels/EnvelopePanel.svelte';
    import LFOPanel from './panels/LFOPanel.svelte';
    import MasterPanel from './panels/MasterPanel.svelte';

    export let currentPreset;
    export let waves;

    
    let activeTab = 'oscillators';
</script>

<div class="synth-controls">
    <div class="tabs">
        <button class="tab-button" class:active={activeTab === 'oscillators'} on:click={() => activeTab = 'oscillators'}>
            OSC
        </button>
        <button class="tab-button" class:active={activeTab === 'filter'} on:click={() => activeTab = 'filter'}>
            FILTER
        </button>
        <button class="tab-button" class:active={activeTab === 'envelope'} on:click={() => activeTab = 'envelope'}>
            ADSR
        </button>
        <button class="tab-button" class:active={activeTab === 'modulation'} on:click={() => activeTab = 'modulation'}>
            LFO
        </button>
        <button class="tab-button" class:active={activeTab === 'master'} on:click={() => activeTab = 'master'}>
            LEVEL
        </button>
    </div>

    {#if activeTab === 'oscillators'}
        <OscillatorPanel bind:currentPreset {waves} />
    {:else if activeTab === 'filter'}
        <FilterPanel bind:currentPreset />
    {:else if activeTab === 'envelope'}
        <EnvelopePanel bind:currentPreset />
    {:else if activeTab === 'modulation'}
        <LFOPanel bind:currentPreset />
    {:else if activeTab === 'master'}
        <MasterPanel bind:currentPreset />
    {/if}
</div>

<style>
    .synth-controls {
        /* width: 100%; */
        width: 300px;
        margin: 0 auto;
        font-family: 'Roboto', sans-serif;
        font-size: 12px;
    }

    .tabs {
        display: flex;
        gap: 4px;
        margin-bottom: 20px;
        width: 100%;
        justify-content: center;
    }

    .tab-button {
        padding: 8px;
        border: none;
        background: #f0f0f0;
        cursor: pointer;
        border-radius: 4px;
        /* transition: background-color 0.2s; */
    }

    .tab-button:hover {
        background: #e0e0e0;
    }

    .tab-button.active {
        background: #4CAF50;
        color: white;
    }
</style> 