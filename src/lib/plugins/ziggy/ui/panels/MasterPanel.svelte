<script>
    export let currentPreset;
</script>

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
            <input type="range" bind:value={currentPreset.masterGain} min={0} max={1.2} step={0.01}>
            <span class="value-display">{(currentPreset.masterGain ?? 0).toFixed(2)}</span>
        </label>
        <label>
            Polyphony:
            <input type="number" bind:value={currentPreset.polyphony} min={1} max={8} step={1}>
        </label>
        <label>
            Distortion:
            <input type="range" bind:value={currentPreset.distortion} min={0} max={1} step={0.01}>
            <span class="value-display">{(currentPreset.distortion ?? 0).toFixed(2)}</span>
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

    input[type="number"] {
        width: 60px;
        text-align: center;
    }
</style> 