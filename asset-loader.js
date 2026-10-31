const decoder = new TextDecoder();

async function verify(bytes, expected, label) {
    const digest = await crypto.subtle.digest('SHA-256', bytes);
    const actual = Array.from(new Uint8Array(digest), byte => byte.toString(16).padStart(2, '0')).join('');
    if (actual !== expected) throw new Error(`Source file changed: ${label}`);
    return bytes;
}

export async function unzip(bytes) {
    const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
    let end = bytes.length - 22;
    const minimum = Math.max(0, end - 65535);
    while (end >= minimum && view.getUint32(end, true) !== 0x06054b50) end--;
    if (end < minimum) throw new Error('Invalid source ZIP archive');
    const count = view.getUint16(end + 10, true);
    let offset = view.getUint32(end + 16, true);
    const entries = new Map();
    for (let index = 0; index < count; index++) {
        if (view.getUint32(offset, true) !== 0x02014b50) throw new Error('Invalid ZIP directory');
        const method = view.getUint16(offset + 10, true);
        const length = view.getUint32(offset + 20, true);
        const size = view.getUint32(offset + 24, true);
        const nameLength = view.getUint16(offset + 28, true);
        const extraLength = view.getUint16(offset + 30, true);
        const commentLength = view.getUint16(offset + 32, true);
        const local = view.getUint32(offset + 42, true);
        const name = decoder.decode(bytes.subarray(offset + 46, offset + 46 + nameLength));
        entries.set(name, async () => {
            if (view.getUint32(local, true) !== 0x04034b50) throw new Error(`Invalid ZIP entry: ${name}`);
            const start = local + 30 + view.getUint16(local + 26, true) + view.getUint16(local + 28, true);
            const compressed = bytes.subarray(start, start + length);
            let result;
            if (method === 0) result = compressed.slice();
            else if (method === 8) {
                const stream = new Blob([compressed]).stream().pipeThrough(new DecompressionStream('deflate-raw'));
                result = new Uint8Array(await new Response(stream).arrayBuffer());
            } else throw new Error(`Unsupported ZIP compression: ${method}`);
            if (result.length !== size) throw new Error(`Incomplete ZIP entry: ${name}`);
            return result;
        });
        offset += 46 + nameLength + extraLength + commentLength;
    }
    return entries;
}

export function createAssetLoader(config, fetcher = fetch) {
    const pending = new Map();
    const archives = new Map();
    const queue = [];
    let active = 0;
    async function download(url, hash) {
        if (active >= 10) await new Promise(resolve => queue.push(resolve));
        active++;
        try {
            let lastError;
            for (let attempt = 0; attempt < 3; attempt++) {
                try {
                    const response = await fetcher(url, {mode: 'cors', credentials: 'omit', signal: AbortSignal.timeout(60000)});
                    if (!response.ok) throw new Error(`HTTP ${response.status}`);
                    const bytes = new Uint8Array(await response.arrayBuffer());
                    return await verify(bytes, hash, url);
                } catch (error) {
                    lastError = error;
                    if (attempt < 2) await new Promise(resolve => setTimeout(resolve, 500 * (attempt + 1)));
                }
            }
            throw new Error(`Unable to load ${url}: ${lastError.message}`);
        } finally {
            active--;
            queue.shift()?.();
        }
    }
    async function readOriginal(path) {
        const entry = config.files[path];
        if (!entry) {
            if (!config.local.includes(path)) throw new Error(`No resource source for ${path}`);
            const response = await fetcher(path);
            if (!response.ok) throw new Error(`Unable to load ${path}: HTTP ${response.status}`);
            return new Uint8Array(await response.arrayBuffer());
        }
        const source = config.sources[entry.source];
        if (source.archive) {
            if (!archives.has(entry.source)) archives.set(entry.source, download(source.archive, source.sha256).then(unzip));
            const entries = await archives.get(entry.source);
            const extract = entries.get(entry.path);
            if (!extract) throw new Error(`Missing archive resource: ${entry.path}`);
            return verify(await extract(), entry.sha256, entry.path);
        }
        const url = source.base + entry.path.split('/').map(encodeURIComponent).join('/');
        return download(url, entry.sha256);
    }
    function read(path) {
        if (!pending.has(path)) pending.set(path, readOriginal(path));
        return pending.get(path);
    }
    return {read, text: async path => decoder.decode(await read(path))};
}
