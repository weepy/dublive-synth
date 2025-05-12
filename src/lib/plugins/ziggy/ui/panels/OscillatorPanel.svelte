<script>
    export let currentPreset;
    export let waves;

    const SHOW_WAVE3 = true;

    function next(prop, _waves, dir = 1) {
        const currentUrl = currentPreset[prop];
        const currentIndex = _waves.findIndex(wave => wave.url === currentUrl);
        const newIndex = (currentIndex + dir + _waves.length) % _waves.length;
        currentPreset[prop] = _waves[newIndex]?.url || _waves[0].url;
        currentPreset = currentPreset;
    }
</script>

<div class="control-group">
    <div class="group-controls">
        <div class="control-row">
            <span class="label">Osc 1</span>
            <div class="wave-selector">
                <button class="wave-btn" on:click={() => next('wave1', waves,-1)}>←</button>
                <select bind:value={currentPreset.wave1}>
                    {#each waves as {url}}
                        <option value={url}>
                            {url.split('/').pop().replace(/\..*$/, '').replace(/[-_]/g, ' ')}
                        </option>
                    {/each}
                </select>
                <button class="wave-btn" on:click={() => next('wave1',waves, 1)}>→</button>
            </div>
        </div>
        <div class="control-row">
            <span class="label">Pitch</span>
            <div class="pitch-control">
                <input type="number" bind:value={currentPreset.oct1} min={-4} max={4} step={1} title="Octave">
                <input type="number" bind:value={currentPreset.semi1} min={-12} max={12} step={1} title="Semitones">
                <input type="number" bind:value={currentPreset.cent1} min={-100} max={100} step={1} title="Cents">
            </div>
        </div>

        <h3></h3>
        <div class="control-row">
            <span class="label">Osc 2</span>
            <button 
                class="toggle-btn" 
                class:active={currentPreset.osc2Enabled === 1}
                on:click={() => currentPreset.osc2Enabled = currentPreset.osc2Enabled === 1 ? 0 : 1}
            >
                {currentPreset.osc2Enabled === 1 ? 'On' : 'Off'}
            </button>
        </div>
        <div class="control-row">
            <span class="label">Wave</span>
            <div class="wave-selector">
                <button class="wave-btn" on:click={() => next('wave2', waves,-1)}>←</button>
                <select bind:value={currentPreset.wave2}>
                    {#each waves as {url}}
                        <option value={url}>
                            {url.split('/').pop().replace(/\..*$/, '').replace(/[-_]/g, ' ')}
                        </option>
                    {/each}
                </select>
                <button class="wave-btn" on:click={() => next('wave2', waves,1)}>→</button>
            </div>
        </div>
        <div class="control-row">
            <span class="label">Pitch</span>
            <div class="pitch-control">
                <input type="number" bind:value={currentPreset.oct2} min={-4} max={4} step={1} title="Octave">
                <input type="number" bind:value={currentPreset.semi2} min={-12} max={12} step={1} title="Semitones">
                <input type="number" bind:value={currentPreset.cent2} min={-100} max={100} step={1} title="Cents">
            </div>
        </div>

        <div class="control-row">
            <span class="label">Mix</span>
            <div class="slider-control">
                <input type="range" bind:value={currentPreset.mix} min={0} max={1} step={0.001}>
                <span class="value-display">{(currentPreset.mix ?? 0).toFixed(2)}</span>
            </div>
        </div>
        <div class="control-row">
            <span class="label">FM Amount</span>
            <div class="slider-control">
                <input type="range" bind:value={currentPreset.fmAmount} min={0} max={1} step={0.01}>
                <span class="value-display">{(currentPreset.fmAmount ?? 0).toFixed(2)}</span>
            </div>
        </div>
        <hr/>

        {#if SHOW_WAVE3}
        <div class="control-row">
            <span class="label">Osc 3</span>
            <button 
                class="toggle-btn" 
                class:active={currentPreset.osc3Enabled === 1}
                on:click={() => currentPreset.osc3Enabled = currentPreset.osc3Enabled === 1 ? 0 : 1}
            >
                {currentPreset.osc3Enabled === 1 ? 'On' : 'Off'}
            </button>
        </div>

        <div class="control-row">
            <span class="label">Wave</span>
            <div class="wave-selector">
                <button class="wave-btn" on:click={() => next('wave3',waves, -1)}>←</button>
                <select bind:value={currentPreset.wave3}>
                    {#each waves as {url}}
                        <option value={url}>
                            {url.split('/').pop().replace(/\..*$/, '').replace(/[-_]/g, ' ')}
                        </option>
                    {/each}
                </select>
                <button class="wave-btn" on:click={() => next('wave3',waves, 1)}>→</button>
            </div>
        </div>

        <div class="control-row">
            <span class="label">Pitch</span>
            <div class="pitch-control">
                <input type="number" bind:value={currentPreset.oct3} min={-4} max={4} step={1} title="Octave">
                <input type="number" bind:value={currentPreset.semi3} min={-12} max={12} step={1} title="Semitones">
                <input type="number" bind:value={currentPreset.cent3} min={-100} max={100} step={1} title="Cents">
            </div>
        </div>
        <div class="control-row">
            <span class="label">Level</span>
            <div class="slider-control">
                <input type="range" bind:value={currentPreset.wave3Level} min={0} max={1} step={0.001}>
                <span class="value-display">{(currentPreset.wave3Level ?? 0).toFixed(3)}</span>
            </div>
        </div>
        <div class="control-row">
            <span class="label">Decay</span>
            <div class="slider-control">
                <input type="range" bind:value={currentPreset.wave3Decay} min={0} max={1} step={0.001}>
                <span class="value-display">{(currentPreset.wave3Decay ?? 0).toFixed(3)}</span>
            </div>
        </div>
        {/if}
    </div>
</div>

<style>
    .control-group {
        background: #f5f5f5;
        padding: 15px;
        border-radius: 8px;
    }

    .group-controls {
        display: flex;
        flex-direction: column;
        gap: 8px;
    }

    .control-row {
        display: flex;
        align-items: center;
        gap: 10px;
    }

    .label {
        min-width: 80px;
        font-weight: 500;
    }

    .slider-control {
        display: flex;
        align-items: center;
        gap: 10px;
        flex: 1;
    }

    input[type="range"] {
        flex: 1;
    }

    .value-display {
        text-align: right;
        font-family: monospace;
        min-width: 45px;
    }

    .wave-selector {
        display: flex;
        align-items: center;
        gap: 5px;
        flex: 1;
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

    .pitch-control {
        display: flex;
        gap: 4px;
        flex: 1;
    }

    .pitch-control input[type="number"] {
        text-align: center;
    }

    .toggle-btn {
        padding: 4px 12px;
        border: 1px solid #ccc;
        border-radius: 4px;
        background: #eee;
        cursor: pointer;
        transition: all 0.2s ease;
    }

    .toggle-btn:hover {
        background: #ddd;
    }

    .toggle-btn.active {
        background: #4CAF50;
        color: white;
        border-color: #45a049;
    }
</style> 