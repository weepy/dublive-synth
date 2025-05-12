<script lang="ts">
    import SavePatchDialog from './SavePatchDialog.svelte';
    
    export let currentPreset: { name: string; [key: string]: any };
    export let patches: { [key: string]: any };
    
    let showSaveDialog = false;

    function savePatch(name: string) {
        if (!name) return;
        currentPreset.name = name;
        patches[name] = { ...currentPreset };
        localStorage.setItem('patches', JSON.stringify(patches));
    }

    function loadPatch(name: string) {
        if (patches[name]) {
            currentPreset = { ...patches[name] };
        }
    }

    function deletePatch(name: string) {
        if (name === 'Default') return; // Prevent deleting default patch
        delete patches[name];
        localStorage.setItem('patches', JSON.stringify(patches));
        if (currentPreset.name === name) {
            currentPreset = { ...patches['Default'] };
        }
    }

    function handleSavePatch(event: CustomEvent) {
        const name = event.detail;
        if (!name) return;
        savePatch(name);
    }
</script>

<div class="patch-controls">
    <select 
        style="width: 200px;"
        value={currentPreset.name}
        on:change={(e) => loadPatch(e.currentTarget.value)}
    >
        {#each Object.keys(patches) as patchName}
            <option value={patchName}>{patchName}</option>
        {/each}
    </select>
    <button on:click={() => showSaveDialog = true}>Save</button>
    <button on:click={() => deletePatch(currentPreset.name)}>Delete</button>
</div>

<SavePatchDialog
    bind:show={showSaveDialog}
    currentName={currentPreset.name}
    {patches}
    on:save={handleSavePatch}
/>

<style>
    .patch-controls {
        display: flex;
        gap: 8px;
        margin-bottom: 16px;
        align-items: center;
        justify-content: center;
    }

    .patch-controls select,
    .patch-controls input {
        padding: 4px 8px;
        border: 1px solid #ccc;
        border-radius: 4px;
    }

    .patch-controls button {
        padding: 4px 12px;
        background: #4CAF50;
        color: white;
        border: none;
        border-radius: 4px;
        cursor: pointer;
    }

    .patch-controls button:hover {
        background: #45a049;
    }
</style> 