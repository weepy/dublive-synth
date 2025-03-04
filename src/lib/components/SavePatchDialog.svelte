<script>
    import { createEventDispatcher } from 'svelte';
    const dispatch = createEventDispatcher();

    export let show = false;
    export let currentName = '';
    export let patches = {};

    let patchName = '';
    let showOverwrite = false;

    $: {
        if (show) {
            patchName = currentName;
            showOverwrite = false;
        }
    }

    function handleSave() {
        if (patches[patchName] && !showOverwrite) {
            showOverwrite = true;
            return;
        }
        
        dispatch('save', patchName);
        close();
    }

    function close() {
        show = false;
        showOverwrite = false;
    }
</script>

{#if show}
    <div class="modal-backdrop">
        <div class="modal">
            {#if !showOverwrite}
                <h2>Save Patch</h2>
                <input
                    type="text"
                    bind:value={patchName}
                    placeholder="Enter patch name"
                    autofocus
                >
                <div class="buttons">
                    <button on:click={handleSave}>Save</button>
                    <button on:click={close}>Cancel</button>
                </div>
            {:else}
                <h2>Overwrite Patch?</h2>
                <p>A patch named "{patchName}" already exists. Do you want to overwrite it?</p>
                <div class="buttons">
                    <button on:click={handleSave}>Yes, Overwrite</button>
                    <button on:click={() => showOverwrite = false}>Back</button>
                    <button on:click={close}>Cancel</button>
                </div>
            {/if}
        </div>
    </div>
{/if}

<style>
    .modal-backdrop {
        position: fixed;
        top: 0;
        left: 0;
        width: 100%;
        height: 100%;
        background: rgba(0, 0, 0, 0.5);
        display: flex;
        justify-content: center;
        align-items: center;
        z-index: 1000;
    }

    .modal {
        background: white;
        padding: 20px;
        border-radius: 8px;
        min-width: 300px;
        box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
    }

    h2 {
        margin: 0 0 20px 0;
    }

    input {
        width: 100%;
        padding: 8px;
        margin-bottom: 20px;
        border: 1px solid #ccc;
        border-radius: 4px;
    }

    .buttons {
        display: flex;
        gap: 10px;
        justify-content: flex-end;
    }

    button {
        padding: 8px 16px;
        border: none;
        border-radius: 4px;
        cursor: pointer;
    }

    button:first-child {
        background: #4CAF50;
        color: white;
    }

    button:not(:first-child) {
        background: #f0f0f0;
    }

    button:hover {
        opacity: 0.9;
    }
</style> 