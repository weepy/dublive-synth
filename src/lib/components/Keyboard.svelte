<script>
    export let onNoteOn;
    export let onNoteOff;
    
    let activeKeys = new Set();

    // Initialize currentOctave from localStorage or default to 4
    let currentOctave = parseInt(localStorage.getItem('currentOctave')) || 2;

    // Update localStorage when currentOctave changes
    $: {
        localStorage.setItem('currentOctave', currentOctave);
    }

    const keyboardLayout = [
        { note: 0, key: 'A', type: 'white' },
        { note: 1, key: 'W', type: 'black' },
        { note: 2, key: 'S', type: 'white' },
        { note: 3, key: 'E', type: 'black' },
        { note: 4, key: 'D', type: 'white' },
        { note: 5, key: 'F', type: 'white' },
        { note: 6, key: 'T', type: 'black' },
        { note: 7, key: 'G', type: 'white' },
        { note: 8, key: 'Y', type: 'black' },
        { note: 9, key: 'H', type: 'white' },
        { note: 10, key: 'U', type: 'black' },
        { note: 11, key: 'J', type: 'white' },
        { note: 12, key: 'K', type: 'white' },
        { note: 13, key: 'O', type: 'black' },
        { note: 14, key: 'L', type: 'white' },
        { note: 15, key: 'P', type: 'black' },
        { note: 16, key: ';', type: 'white' },
        

    ];

    function handleKeyDown(e) {
        const key = keyboardLayout.find(k => k.key === e.key.toUpperCase());
        if (key && !e.repeat && !activeKeys.has(key.note)) {
            const keyElement = document.querySelector(`[data-note="${key.note}"]`);
            if (keyElement) {
                onNoteOn(key.note + currentOctave * 12, keyElement);
            }
        }
    }

    function handleKeyUp(e) {

        if(e.key.toLowerCase() === 'z') {
            currentOctave = Math.max(0, currentOctave - 1);
            return;
        }
        if(e.key.toLowerCase() === 'x') {
            currentOctave = Math.min(8, currentOctave + 1);
            return;
        }

        const key = keyboardLayout.find(k => k.key === e.key.toUpperCase());
        if (key) {
            const keyElement = document.querySelector(`[data-note="${key.note}"]`);
            if (keyElement) {
                onNoteOff(key.note + currentOctave * 12, keyElement);
            }
        }
    }

    function handlePointerDown(e) {
        e.preventDefault();
        const note = parseInt(e.currentTarget.dataset.note);
        if (!activeKeys.has(note)) {
            onNoteOn(note + currentOctave * 12, e.currentTarget);
            activeKeys.add(note);
            // Capture the pointer to ensure we get the pointerup even if dragged outside
            e.currentTarget.setPointerCapture(e.pointerId);
        }
    }

    function handlePointerUp(e) {
        const note = parseInt(e.currentTarget.dataset.note);
        if (activeKeys.has(note)) {
            onNoteOff(note + currentOctave * 12, e.currentTarget);
            activeKeys.delete(note);
            e.currentTarget.releasePointerCapture(e.pointerId);
        }
    }
</script>

<div class="keyboard">
    {#each keyboardLayout as { note, key, type }}
        <div
            class="key {type === 'black' ? 'black-key' : ''}"
            data-note={note}
            on:pointerdown={handlePointerDown}
            on:pointerup={handlePointerUp}
            on:pointercancel={handlePointerUp}
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
        touch-action: none;
        -webkit-user-select: none;
        user-select: none;
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
</style> 