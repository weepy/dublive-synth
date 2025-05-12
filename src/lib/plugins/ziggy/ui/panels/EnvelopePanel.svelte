<script>
    export let currentPreset;
</script>

<div class="control-group">
    <div class="group-controls">
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
</style> 