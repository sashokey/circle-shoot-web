import assert from 'node:assert/strict';
import { existsSync, readFileSync } from 'node:fs';
import { createRequire } from 'node:module';
import { dirname, resolve } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import vm from 'node:vm';
import { createAssetLoader } from '../asset-loader.js';

const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const sources = JSON.parse(readFileSync(resolve(root, 'asset-sources.json'), 'utf8'));
const cues = JSON.parse(readFileSync(resolve(root, 'music-cues.json'), 'utf8'));
const orders = ['0', '24', '28', '32', '34', '36', '38', '39'];
assert.deepEqual(Object.keys(cues), orders);
assert.ok(!existsSync(resolve(root, 'content/music')));
assert.ok(!sources.local.some(path => path.startsWith('content/music/')));
const loader = createAssetLoader(sources);
const bytes = await loader.read('content/music/zuma.mo3');
const rendered = new Set();
let complete = false;
const context = vm.createContext({
    console, process, require: createRequire(import.meta.url), module: {exports: {}},
    __dirname: resolve(root, 'vendor/libopenmpt'),
    __filename: resolve(root, 'vendor/libopenmpt/libopenmpt.js'),
    TextDecoder, TextEncoder, WebAssembly, URL, performance, setTimeout, clearTimeout,
    location: {href: pathToFileURL(resolve(root, 'music-worker.js')).href},
    postMessage: message => {
        if (message.error) throw new Error(message.error);
        if (message.done) { complete = true; return; }
        const wav = Buffer.from(message.wav);
        assert.equal(wav.toString('ascii', 0, 4), 'RIFF');
        assert.equal(wav.toString('ascii', 8, 12), 'WAVE');
        assert.equal(wav.readUInt16LE(22), 2);
        assert.equal(wav.readUInt32LE(24), 44100);
        assert.equal(wav.readUInt16LE(34), 16);
        assert.equal(wav.readUInt32LE(40), wav.length - 44);
        const duration = (wav.length - 44) / (44100 * 4);
        assert.ok(Math.abs(duration - cues[message.order].duration) < 1 / 44100);
        let peak = 0;
        for (let offset = 44; offset < wav.length; offset += 2) peak = Math.max(peak, Math.abs(wav.readInt16LE(offset)));
        assert.ok(peak > 1000, `Music cue ${message.order} is silent`);
        rendered.add(message.order);
        console.log(`PASS: music cue ${message.order}, ${duration.toFixed(3)} seconds, audible PCM.`);
    }
});
context.self = context;
context.importScripts = path => {
    assert.equal(path, 'vendor/libopenmpt/libopenmpt.js');
    vm.runInContext(readFileSync(resolve(root, path), 'utf8'), context);
};
vm.runInContext(readFileSync(resolve(root, 'music-worker.js'), 'utf8'), context);
await context.onmessage({data: {bytes, cues}});
assert.ok(complete);
assert.deepEqual([...rendered], orders);
console.log('PASS: all eight external music subsongs render with their original start orders and durations.');
