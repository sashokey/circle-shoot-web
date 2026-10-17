const encoder = new TextEncoder();
const decoder = new TextDecoder();

export async function createEngine(bytes, host) {
    let wasm;
    const read = pointer => {
        const data = new Uint8Array(wasm.memory.buffer);
        let end = pointer;
        while (end < data.length && data[end]) ++end;
        return decoder.decode(data.subarray(pointer, end));
    };
    const view = () => new DataView(wasm.memory.buffer);
    const web = {};
    for (const name of ['surface','release','image','shape','polygon','pixels','audio','control']) web[name] = (...args) => host[name]?.(...args);
    web.text = (target, font, text, ...args) => host.text(target, font, read(text), ...args);
    web.width = (font, text) => host.width(font, read(text));
    web.message = pointer => { throw new Error(read(pointer)); };
    web.store = (path, pointer, length) => host.store(read(path), new Uint8Array(wasm.memory.buffer, pointer, length).slice());
    const wasi = {
        environ_get: () => 0,
        environ_sizes_get: (count, size) => { view().setUint32(count, 0, true); view().setUint32(size, 0, true); return 0; },
        clock_time_get: (id, precision, pointer) => { view().setBigUint64(pointer, BigInt(Date.now()) * 1000000n, true); return 0; },
        fd_close: () => 8,
        fd_prestat_get: () => 8,
        fd_prestat_dir_name: () => 8,
        fd_seek: () => 8,
        fd_write: (fd, vectors, count, written) => {
            let length = 0;
            for (let i = 0; i < count; ++i) {
                const pointer = view().getUint32(vectors + i * 8, true);
                const size = view().getUint32(vectors + i * 8 + 4, true);
                host.log?.(decoder.decode(new Uint8Array(wasm.memory.buffer, pointer, size)));
                length += size;
            }
            view().setUint32(written, length, true);
            return 0;
        },
        proc_exit: status => { throw new Error('WebAssembly exit: ' + status); }
    };
    const module = await WebAssembly.compile(bytes);
    wasm = (await WebAssembly.instantiate(module, {web, wasi_snapshot_preview1:wasi})).exports;
    host.setMemory?.(wasm.memory);
    wasm._initialize();
    wasm.web_prepare();
    const withString = (value, action) => {
        const data = encoder.encode(value + '\0');
        const pointer = wasm.malloc(data.length);
        new Uint8Array(wasm.memory.buffer, pointer, data.length).set(data);
        try { return action(pointer); } finally { wasm.free(pointer); }
    };
    return {
        wasm,
        registerImage: (item,id) => withString(item.name, name => wasm.web_resource(name,id,item.width,item.height,item.rows || 1,item.cols || 1)),
        registerFont: (name,id,font) => withString(name,pointer => wasm.web_font(pointer,id,font.height ?? font,font.ascent ?? Math.trunc(font*0.8),font.padding ?? 0,font.lineSpacing ?? font+2)),
        soundId: name => withString(name,pointer => wasm.web_sound(pointer)),
        registerFile: (path,data) => withString(path,name => {
            const pointer = wasm.malloc(data.length || 1);
            new Uint8Array(wasm.memory.buffer,pointer,data.length).set(data);
            wasm.web_register_file(name,pointer,data.length);
            wasm.free(pointer);
        }),
        init: seed => wasm.web_init(seed),
        start: (stage=0,practice='',difficulty=0,endless=false) => withString(practice,pointer => wasm.web_start(stage,pointer,difficulty,Number(endless))),
        resume: practice => wasm.web_resume(Number(practice)),
        tick: count => wasm.web_tick(count),
        draw: () => wasm.web_draw(),
        pointer: (x,y) => wasm.web_pointer(Math.round(x),Math.round(y)),
        click: (x,y,button=0) => wasm.web_click(Math.round(x),Math.round(y),button),
        press: (x,y,button=0) => wasm.web_press(Math.round(x),Math.round(y),button),
        release: (x,y,button=0) => wasm.web_release(Math.round(x),Math.round(y),button),
        key: (key,character=0) => wasm.web_key(key,character),
        pause: value => wasm.web_pause(Number(value)),
        continue: menu => wasm.web_continue(Number(menu)),
        save: () => wasm.web_save(),
        snapshot: () => JSON.parse(read(wasm.web_snapshot())),
        balls: () => JSON.parse(read(wasm.web_balls())),
        levels: () => JSON.parse(read(wasm.web_levels())),
        qa: (action,value=0) => wasm.web_qa(action,value)
    };
}
