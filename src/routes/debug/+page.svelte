<script>
    import { onMount } from 'svelte';

    
    let audioContext;
    let isPlaying = false;

    
    let synthNode;

    // Initialize audio context with suspended state for iOS compatibility
    function createAudioContext() {
        return new (window.AudioContext || window.webkitAudioContext)();
    }

    async function init() {
        // Only create context if it doesn't exist
        if (!audioContext) {
            audioContext = createAudioContext();
            await audioContext.audioWorklet.addModule("/worklets/synth_worklet_debug.js");
            synthNode = new AudioWorkletNode(audioContext, 'synth-processor', {
                numberOfOutputs: 1,
                outputChannelCount: [2]
            });
            
            // Add error handler
            synthNode.onprocessorerror = (err) => {
                console.error('AudioWorklet processor error:', err);
            };

            // Add message handler
            synthNode.port.onmessage = (event) => {
                console.log('Message from AudioWorklet:', event.data);
            };

            synthNode.connect(audioContext.destination);

            // Send a test message to verify communication
            synthNode.port.postMessage({ type: 'test' });
        }

        // Ensure we resume the context on iOS
        if (audioContext.state === 'suspended') {
            await audioContext.resume();
        }
    }



    async function togglePlay() {
        if (!audioContext) {
            await init();
        }
        isPlaying = !isPlaying;
        if (isPlaying) {
            await audioContext.resume();
            console.log('Audio Context State:', audioContext.state);
        } else {
            await audioContext.suspend();
            console.log('Audio Context State:', audioContext.state);
        }
    }
</script>


<button class="play-button" on:click={togglePlay}>
    {isPlaying ? 'Stop' : 'Play'}
</button>
