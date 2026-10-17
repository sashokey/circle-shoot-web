import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { createAssetLoader } from '../asset-loader.js';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const manifest = JSON.parse(readFileSync(process.argv[2] || resolve(root, 'assets.json'), 'utf8'));
const images = new Map(manifest.images.map(image => [image.name, image]));
const xml = readFileSync(resolve(root, 'content/levels/levels.xml'), 'utf8').replace(/<!--[\s\S]*?-->/g, '');
const attributes = text => Object.fromEntries([...text.matchAll(/([\w-]+)\s*=\s*"([^"]*)"/g)].map(match => [match[1], match[2]]));
const sources = JSON.parse(readFileSync(resolve(root, 'asset-sources.json'), 'utf8'));
const loader = createAssetLoader(sources, (url, options) => /^https:\/\//.test(url) ? fetch(url, options) : Promise.resolve(new Response(readFileSync(resolve(root, url)))));
const resources = new Map(await Promise.all(Object.keys(sources.files).map(async path => [path, Buffer.from(await loader.read(path))])));
const curveFiles = manifest.files.filter(file => file.endsWith('.dat'));
const fonts = JSON.parse(readFileSync(resolve(root, 'font-metrics.json'), 'utf8'));
const required = ['content/music/zuma.mo3', ...manifest.files, ...Object.values(manifest.sounds), ...manifest.images.flatMap(image => image.parts.flatMap(part => [part.file, ...(part.mask ? [part.mask] : [])])), ...Object.values(fonts).flatMap(font => font.layers.map(layer => layer.image))];
for (const path of required) assert.ok(resources.has(path) || sources.local.includes(path), `No source for ${path}`);
console.log(`PASS: ${resources.size} external resources verified against SHA-256 hashes; all resource references resolve.`);
const errors = [];
let curveCount = 0;
let tunnelCount = 0;
let maskCount = 0;
const graphics = new Map();

function tunnelSpans(file) {
    const data = resources.get(file);
    assert.equal(data.toString('ascii', 0, 4), 'CURV', file);
    let offset = 4;
    const byte = () => data.readUInt8(offset++);
    const uint = () => { const value = data.readUInt32LE(offset); offset += 4; return value; };
    const version = uint();
    assert.ok(version >= 1 && version <= 3, file);
    const small = version > 2 ? byte() : false;
    const hasTunnels = version > 2 ? byte() : true;
    if (!small) { uint(); const size = uint(); offset += size; }
    const count = uint();
    let spans = 0;
    let previous = false;
    for (let index = 0; index < count; index++) {
        offset += version === 1 || index === 0 ? 8 : 2;
        const tunnel = hasTunnels ? byte() !== 0 : false;
        if (hasTunnels) byte();
        if (tunnel && !previous) spans++;
        previous = tunnel;
    }
    assert.equal(offset, data.length, file);
    return spans;
}

function dimensions(file) {
    const data = resources.get(file);
    if (data.toString('ascii', 0, 3) === 'GIF') return [data.readUInt16LE(6), data.readUInt16LE(8)];
    if (data.subarray(0, 8).equals(Buffer.from([137, 80, 78, 71, 13, 10, 26, 10]))) return [data.readUInt32BE(16), data.readUInt32BE(20)];
    throw new Error(`Unsupported foreground image: ${file}`);
}

for (const match of xml.matchAll(/<Graphics\b([^>]*)>([\s\S]*?)<\/Graphics>/g)) {
    const graphic = attributes(match[1]);
    graphics.set(graphic.id, graphic);
    let spans = 0;
    for (const key of ['curve', 'curve2', 'curve3']) {
        if (!graphic[key]) continue;
        const curve = curveFiles.find(file => file.replaceAll('\\', '/').endsWith('/' + graphic[key] + '.dat'));
        assert.ok(curve, `Missing curve: ${graphic[key]}`);
        spans += tunnelSpans(curve);
        curveCount++;
    }
    const cutouts = [...match[2].matchAll(/<Cutout\b([^>]*)\/>/g)].map(item => attributes(item[1]));
    if (spans && !cutouts.length) errors.push(`${graphic.id}: ${spans} tunnel spans have no foreground masks`);
    tunnelCount += spans;
    for (const cutout of cutouts) {
        maskCount++;
        const name = `levels/${graphic.id}/${cutout.image.toLowerCase()}`;
        try {
            const image = images.get(name);
            assert.ok(image, 'Missing manifest entry');
            assert.ok(image.width > 1 && image.height > 1, 'Placeholder dimensions');
            assert.ok(image.parts.length > 0, 'Empty foreground mask');
            assert.ok(Number(cutout.pri) > 0, 'Foreground must be above tunnel balls');
            for (const part of image.parts) {
                const [width, height] = dimensions(part.file);
                const [x, y, w, h] = part.source;
                assert.ok(x >= 0 && y >= 0 && w > 0 && h > 0 && x + w <= width && y + h <= height, 'Source rectangle exceeds image bounds');
                if (part.mask) {
                    assert.deepEqual(dimensions(part.mask), part.cropBeforeMask ? [w, h] : [width, height], 'Alpha mask dimensions do not match');
                }
            }
        } catch (error) {
            errors.push(`${name}: ${error.message}`);
        }
    }
}

const progression = attributes(xml.match(/<StageProgression\b([\s\S]*?)\/>/)[1]);
let campaignCount = 0;
for (const [key, value] of Object.entries(progression)) {
    if (!/^stage\d+$/.test(key)) continue;
    for (const name of value.split(',')) {
        assert.ok(graphics.has(name), `Unknown campaign layout: ${name}`);
        campaignCount++;
    }
}
if (errors.length) {
    for (const error of errors) console.error(error);
    process.exitCode = 1;
} else {
    console.log(`PASS: ${campaignCount} campaign levels, ${graphics.size} layouts, ${curveCount} curves, ${tunnelCount} tunnel spans, ${maskCount} foreground masks.`);
}
