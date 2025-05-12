<script>
    export let currentPreset;
</script>

<div class="control-group">
    <div class="group-controls">
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
            <input type="range" bind:value={currentPreset.lfoAmount} min={-1} max={1} step={0.01}>
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

    select {
        width: 120px;
    }
</style> 