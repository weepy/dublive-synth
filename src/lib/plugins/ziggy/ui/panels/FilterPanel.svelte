<script>
    export let currentPreset;
</script>

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
            Key Tracking:
            <input type="range" bind:value={currentPreset.filterKeyTracking} min={0} max={2} step={0.01}>
            <span class="value-display">{(currentPreset.filterKeyTracking ?? 1).toFixed(2)}</span>
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