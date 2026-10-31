let initialize;
let initializationFailed;
const initialized = new Promise((resolve, reject) => {
    initialize = resolve;
    initializationFailed = reject;
});
var libopenmpt = {
    locateFile: path => new URL('vendor/libopenmpt/' + path, self.location.href).href,
    onRuntimeInitialized: () => initialize(),
    onAbort: reason => initializationFailed(new Error(String(reason)))
};
importScripts('vendor/libopenmpt/libopenmpt.js');

function renderCue(handle, order, cue, output) {
    if (!Module._openmpt_module_select_subsong(handle, cue.subsong)) throw new Error(`Missing music subsong ${cue.subsong}`);
    if (Module._openmpt_module_get_current_order(handle) !== Number(order)) throw new Error(`Unexpected start order for music cue ${order}`);
    const duration = Module._openmpt_module_get_duration_seconds(handle);
    if (Math.abs(duration - cue.duration) > 0.01) throw new Error(`Unexpected duration for music cue ${order}`);
    Module._openmpt_module_set_repeat_count(handle, 0);
    const sampleRate = 44100;
    const frames = Math.round(cue.duration * sampleRate);
    const wav = new Uint8Array(44 + frames * 4);
    const header = new DataView(wav.buffer);
    const tag = (offset, text) => wav.set(new TextEncoder().encode(text), offset);
    tag(0, 'RIFF');
    header.setUint32(4, wav.length - 8, true);
    tag(8, 'WAVEfmt ');
    header.setUint32(16, 16, true);
    header.setUint16(20, 1, true);
    header.setUint16(22, 2, true);
    header.setUint32(24, sampleRate, true);
    header.setUint32(28, sampleRate * 4, true);
    header.setUint16(32, 4, true);
    header.setUint16(34, 16, true);
    tag(36, 'data');
    header.setUint32(40, frames * 4, true);
    let position = 0;
    while (position < frames) {
        const count = Module._openmpt_module_read_interleaved_stereo(handle, sampleRate, Math.min(4096, frames - position), output);
        if (!count) throw new Error(`Music cue ${order} ended early`);
        wav.set(HEAPU8.subarray(output, output + count * 4), 44 + position * 4);
        position += count;
    }
    return wav.buffer;
}

self.onmessage = async ({data}) => {
    let pointer = 0;
    let handle = 0;
    let output = 0;
    try {
        await initialized;
        const bytes = new Uint8Array(data.bytes);
        pointer = Module._malloc(bytes.length);
        if (!pointer) throw new Error('Unable to allocate music input');
        HEAPU8.set(bytes, pointer);
        handle = Module._openmpt_module_create_from_memory2(pointer, bytes.length, 0, 0, 0, 0, 0, 0, 0);
        if (!handle) throw new Error('Unable to decode external music');
        output = Module._malloc(4096 * 4);
        if (!output) throw new Error('Unable to allocate music output');
        for (const [order, cue] of Object.entries(data.cues)) {
            const wav = renderCue(handle, order, cue, output);
            self.postMessage({order, wav}, [wav]);
        }
        self.postMessage({done: true});
    } catch (error) {
        self.postMessage({error: error.message});
    } finally {
        if (output) Module._free(output);
        if (handle) Module._openmpt_module_destroy(handle);
        if (pointer) Module._free(pointer);
    }
};
