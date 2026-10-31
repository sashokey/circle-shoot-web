import {createAssetLoader} from './asset-loader.js';
import {prepareMusic} from './music-loader.js';
let assets;

import {createEngine} from './engine.js';

import {measureFont,fontCommands} from './bitmap-font.js';

const $ = selector => document.querySelector(selector);

const canvas = $('#game');

const context = canvas.getContext('2d',{alpha:false});

const surfaces = new Map([[0,{canvas,context,width:640,height:480}]]);

const bitmaps = new Map();

const fonts = new Map();

const sounds = new Map();

const audioBuffers = new Map();

const activeSounds = new Map();

const tintCache = new Map();

const drawIssues = new Map();
const cursorCache = new Map();
let cursorKind=0,customCursor=false;

const params = new URLSearchParams(location.search);

const namespace = params.has('qa')?'zuma-deluxe-wasm-qa-v1/':'zuma-deluxe-wasm-v1/';

let engine,memory,nextSurface=1,audioContext,masterGain,music,musicOrder=-1,cues,muted=false,loaded=false,musicVolume=0.38,sfxVolume=0.7;

let lastFrame=0,accumulator=0,lastUI='',levels=[],pointer={x:320,y:400},lastSave=0,altHeld=false;

function surface(width,height) {

    const element = document.createElement('canvas');

    element.width = Math.max(1,Math.ceil(width*2));

    element.height = Math.max(1,Math.ceil(height*2));

    const context = element.getContext('2d',{willReadFrequently:true});

    context.scale(2,2);

    const id = nextSurface++;

    surfaces.set(id,{canvas:element,context,width,height});

    return id;

}

function color(value) {

    return {red:(value>>>16)&255,green:(value>>>8)&255,blue:value&255,alpha:(value>>>24)/255};

}

function begin(target,value,clip) {

    const context = surfaces.get(target)?.context;

    if (!context) return null;

    const rgba = color(value);

    context.save();

    context.globalAlpha = rgba.alpha;

    context.fillStyle = context.strokeStyle = `rgb(${rgba.red},${rgba.green},${rgba.blue})`;

    if (clip?.[2]>0 && clip[3]>0) {

        context.beginPath();

        context.rect(...clip);

        context.clip();

    }

    return context;

}

function tinted(id,value) {

    const item = surfaces.get(id);

    if ((value & 0xffffff) === 0xffffff) return item.canvas;

    const key = id + ':' + (value & 0xffffff);

    if (tintCache.has(key)) return tintCache.get(key);

    const element = document.createElement('canvas');

    element.width = item.canvas.width; element.height = item.canvas.height;

    const context = element.getContext('2d');

    context.drawImage(item.canvas,0,0);

    const rgba = color(value);

    context.globalCompositeOperation = 'multiply';

    context.fillStyle = `rgb(${rgba.red},${rgba.green},${rgba.blue})`;

    context.fillRect(0,0,element.width,element.height);

    context.globalCompositeOperation = 'destination-in';

    context.drawImage(item.canvas,0,0);

    tintCache.set(key,element);

    if (tintCache.size>24) tintCache.delete(tintCache.keys().next().value);

    return element;

}

function image(target,id,x,y,width,height,sx,sy,sw,sh,angle,cx,cy,value,mode,...clip) {

    const source = surfaces.get(id);

    if (!source || sw<=0 || sh<=0 || width<=0 || height<=0) return;

    if (sx < -0.01 || sy < -0.01 || sx+sw>source.width+0.01 || sy+sh>source.height+0.01) {

        const issue={name:source.name || id,source:[sx,sy,sw,sh],size:[source.width,source.height]};

        drawIssues.set(JSON.stringify(issue),issue);

    }

    const context = begin(target,value,clip);

    if (!context) return;

    if (mode===1) context.globalCompositeOperation='lighter';

    context.translate(x+cx,y+cy);

    context.rotate(-angle);

    context.drawImage(tinted(id,value),sx*2,sy*2,sw*2,sh*2,-cx,-cy,width,height);

    context.restore();

}

function pixels(id,pointer,width,height,write) {

    const item=surfaces.get(id);

    if (!item) return;

    const temporary=document.createElement('canvas');

    temporary.width=width; temporary.height=height;

    const context=temporary.getContext('2d',{willReadFrequently:true});

    const words=new Uint32Array(memory.buffer,pointer,width*height);

    if (write) {

        const data=context.createImageData(width,height);

        for(let i=0;i<words.length;++i) {

            data.data[i*4]=(words[i]>>>16)&255;

            data.data[i*4+1]=(words[i]>>>8)&255;

            data.data[i*4+2]=words[i]&255;

            data.data[i*4+3]=words[i]>>>24;

        }

        context.putImageData(data,0,0);

        item.context.clearRect(0,0,item.width,item.height);

        item.context.drawImage(temporary,0,0,item.width,item.height);

        for(const key of tintCache.keys()) if(key.startsWith(id+':')) tintCache.delete(key);

    } else {

        context.drawImage(item.canvas,0,0,width,height);

        const data=context.getImageData(0,0,width,height).data;

        for(let i=0;i<words.length;++i) words[i]=(data[i*4+3]<<24)|(data[i*4]<<16)|(data[i*4+1]<<8)|data[i*4+2];

    }

}

function glyphs(font,text,action) {

    const item=fonts.get(font);

    if (!item) return text.length*10;

    return measureFont(item,text);

}

function text(target,font,text,x,y,value,...clip) {

    const context=begin(target,value,clip);

    if (!context) return;

    const item=fonts.get(font);

    if(item) for(const command of fontCommands(item,text,x,y,color(value))) {

        const [sx,sy,width,height]=command.rect;

        context.globalAlpha=color(command.color).alpha;

        context.drawImage(tinted(command.surface,command.color),sx*2,sy*2,width*2,height*2,command.x,command.y,width,height);

    }

    context.restore();

}

function store(path,data) {

    try {

        const key=namespace+path.toLowerCase();

        if(!data.length) { localStorage.removeItem(key); return; }

        let string='';

        for(let offset=0;offset<data.length;offset+=8192) string+=String.fromCharCode(...data.subarray(offset,offset+8192));

        localStorage.setItem(key,btoa(string));

    } catch { $('#status').textContent='Browser storage is unavailable. Keep this tab open to continue your game.'; }

}

async function activateAudio() {

    if (!audioContext) {

        audioContext=new AudioContext();

        masterGain=audioContext.createGain(); masterGain.connect(audioContext.destination);

        masterGain.gain.value=muted ? 0 : sfxVolume;

        for(const [id,path] of sounds) {

            assets.read(path).then(bytes=>bytes.slice().buffer).then(bytes=>audioContext.decodeAudioData(bytes)).then(buffer=>audioBuffers.set(id,buffer)).catch(()=>{});

        }

    }

    await audioContext.resume();

    if (music && !muted) music.play().catch(()=>{});

}

function audio(op,id,asset,volume,pitch,loop,pan) {

    if (op===3) {

        if (musicOrder===pitch) return;

        musicOrder=pitch;

        music?.pause();

        const cue=cues?.[String(pitch)];

        if(cue) {

            music=new Audio(cue.file); music.loop=true; music.volume=muted ? 0 : musicVolume;

            if(audioContext) music.play().catch(()=>{});

        }

        return;

    }

    const active=activeSounds.get(id);

    if (op===1) { active?.source.stop(); activeSounds.delete(id); return; }

    if (op===2) { if(active) active.gain.gain.value=Math.max(0,volume); return; }

    if (op!==0 || !audioContext || !audioBuffers.has(asset)) return;

    active?.source.stop();

    const source=audioContext.createBufferSource();

    source.buffer=audioBuffers.get(asset); source.loop=Boolean(loop); source.playbackRate.value=2**(pitch/12);

    const gain=audioContext.createGain(); gain.gain.value=Math.max(0,volume);

    const stereo=audioContext.createStereoPanner(); stereo.pan.value=Math.max(-1,Math.min(1,pan/10000));

    source.connect(gain).connect(stereo).connect(masterGain);

    const sound={source,gain}; activeSounds.set(id,sound);

    source.onended=()=>{if(activeSounds.get(id)===sound) activeSounds.delete(id);};

    source.start();

}

function updateCursor() {
    let cursor=cursorKind===1?'pointer':cursorKind===2?'grabbing':cursorKind===3?'text':'default';
    if(customCursor) {
        const name=['IMAGE_CURSOR_POINTER','IMAGE_CURSOR_HAND','IMAGE_CURSOR_DRAGGING','IMAGE_CURSOR_TEXT'][cursorKind] || 'IMAGE_CURSOR_POINTER';
        const item=[...surfaces.values()].find(item=>item.name===name);
        if(item) {
            if(!cursorCache.has(name)) {
                const image=document.createElement('canvas');image.width=item.width;image.height=item.height;image.getContext('2d').drawImage(item.canvas,0,0,image.width,image.height);cursorCache.set(name,image.toDataURL());
            }
            cursor=`url(${cursorCache.get(name)}) ${Math.floor(item.width/2)} ${Math.floor(item.height/2)}, ${cursor}`;
        }
    }
    canvas.style.cursor=cursor;$('#stage').style.cursor=cursor;
}
const host={

    surface, image, pixels, text, audio, store,

    release:id=>{surfaces.delete(id);for(const key of tintCache.keys()) if(key.startsWith(id+':')) tintCache.delete(key);},

    width:(font,text)=>glyphs(font,text),

    setMemory:value=>{memory=value;},

    control:(operation,value)=>{

        if(operation===0) {cursorKind=value;updateCursor();}

        if(operation===1) {musicVolume=value;if(music)music.volume=muted?0:value;}

        if(operation===2) {sfxVolume=value;if(masterGain)masterGain.gain.value=muted?0:value;}

        if(operation===3 && Boolean(document.fullscreenElement)!==Boolean(value)) {if(value)$('#player').requestFullscreen().catch(()=>{});else document.exitFullscreen().catch(()=>{});}

        if(operation===4) {customCursor=Boolean(value);updateCursor();}

    },

    shape:(target,type,x,y,width,height,value,...clip)=>{

        const context=begin(target,value,clip); if(!context) return;

        if(type===0) context.fillRect(x,y,width,height);

        else if(type===1) context.strokeRect(x+.5,y+.5,width,height);

        else if(type===2) {context.beginPath();context.moveTo(x,y);context.lineTo(width,height);context.stroke();}

        else context.clearRect(x,y,width,height);

        context.restore();

    },

    polygon:(target,pointer,count,value)=>{

        const context=begin(target,value); if(!context) return;

        const points=new Int32Array(memory.buffer,pointer,count*2);

        context.beginPath();for(let i=0;i<count;++i) {if(i===0) context.moveTo(points[i*2],points[i*2+1]);else context.lineTo(points[i*2],points[i*2+1]);}

        context.closePath();context.fill();context.restore();

    },

    log:message=>console.warn(message)

};

function fail(error) {

    loaded=false;

    $('#loading').hidden=false;

    $('#load-message').textContent='Unable to load the game: '+error.message;

    $('#progress').hidden=true;

    console.error(error);

}

async function loadBitmap(path) {

    if(bitmaps.has(path)) return bitmaps.get(path);

    const promise=(async()=>{

        const bytes=await assets.read(path);
        const url=URL.createObjectURL(new Blob([bytes]));
        try {const image=new Image();image.src=url;await image.decode();return image;} finally {URL.revokeObjectURL(url);}

    })();

    bitmaps.set(path,promise);return promise;

}

function ui(state) {

    const screen=state.screen;

    $('#pause').disabled=screen!=='game' && screen!=='pause';

    $('#swap').disabled=screen!=='game';

    $('#pause').textContent=screen==='pause'?'Resume':'Pause';

    const signature=JSON.stringify(state.controls);

    if(signature!==lastUI) {

        $('#native-controls').replaceChildren(...state.controls.map(control=>{

            const button=document.createElement('button');

            button.textContent=control.label;

            button.disabled=control.disabled;

            button.style.left=control.x/640*100+'%';

            button.style.top=control.y/480*100+'%';

            button.style.width=control.width/640*100+'%';

            button.style.height=control.height/480*100+'%';

            button.addEventListener('click',event=>{if(event.detail===0 && loaded && !altHeld) {activateAudio();engine.click(control.x+control.width/2,control.y+control.height/2);render();}});

            return button;

        }));

        lastUI=signature;

    }

    $('#status').textContent=state.title?`${state.practice?'Gauntlet':'Adventure'} · ${state.stage+1}-${state.level+1} · ${state.title} · Score ${state.score.toLocaleString('en-US')}`:'Aim · Left click: Shoot · Right click: Swap · Space: Pause · T: Timer';

}

function render() {

    context.setTransform(2,0,0,2,0,0);context.clearRect(0,0,640,480);

    engine.draw();

    ui(engine.snapshot());

}

function frame(time) {

    if(!loaded) return;

    if(altHeld) {lastFrame=0;accumulator=0;requestAnimationFrame(frame);return;}

    try {

        if(lastFrame) accumulator+=Math.min(200,time-lastFrame);

        lastFrame=time;

        const ticks=Math.floor(accumulator/10);

        if(ticks) {engine.tick(ticks);accumulator-=ticks*10;}

        render();

        if(time-lastSave>15000) {engine.save();lastSave=time;}

        requestAnimationFrame(frame);

    } catch(error) {fail(error);}

}

function point(event) {

    const rect=canvas.getBoundingClientRect();

    pointer={x:(event.clientX-rect.left)*640/rect.width,y:(event.clientY-rect.top)*480/rect.height};

    engine.pointer(pointer.x,pointer.y);

}

function action(func) { return ()=>{if(!loaded || altHeld) return;activateAudio();try {func();render();canvas.focus({preventScroll:true});} catch(error) {fail(error);}}; }

$('#stage').addEventListener('pointermove',event=>{if(loaded && !altHeld) point(event);});

$('#stage').addEventListener('pointerdown',event=>{

    if(!loaded || altHeld || !$('#loading').hidden)return;

    event.preventDefault();activateAudio();point(event);engine.press(pointer.x,pointer.y,event.button);

    $('#stage').setPointerCapture(event.pointerId);canvas.focus({preventScroll:true});render();

});

$('#stage').addEventListener('pointerup',event=>{

    if(!loaded || altHeld)return;

    point(event);engine.release(pointer.x,pointer.y,event.button);

    if($('#stage').hasPointerCapture(event.pointerId))$('#stage').releasePointerCapture(event.pointerId);

    render();

});

$('#stage').addEventListener('pointercancel',event=>{if(loaded && !altHeld)engine.release(-1,-1,event.button);});

$('#stage').addEventListener('contextmenu',event=>event.preventDefault());

document.addEventListener('keydown',event=>{

    if(!loaded || ['SELECT','INPUT'].includes(event.target.tagName)) return;

    if(event.key==='Alt' && engine.snapshot().screen==='game') {event.preventDefault();altHeld=true;return;}
    if(altHeld) {event.preventDefault();return;}

    if(engine.snapshot().inputFocus) {
        event.preventDefault();engine.key(event.key.length===1?0:event.keyCode,event.key.length===1?event.key.charCodeAt(0):0);render();return;
    }
    if(event.code==='Space') {event.preventDefault();const state=engine.snapshot();engine.pause(!state.paused);render();}

    else if(event.code==='Escape') {event.preventDefault();engine.key(27);render();}

    else if(event.key.length===1) engine.key(0,event.key.charCodeAt(0));

});

document.addEventListener('keyup',event=>{if(altHeld && event.key==='Alt') {altHeld=event.altKey;lastFrame=0;accumulator=0;}});
window.addEventListener('blur',()=>{altHeld=false;lastFrame=0;accumulator=0;});

document.addEventListener('visibilitychange',()=>{if(loaded && document.hidden) {altHeld=false;engine.pause(true);engine.save();lastFrame=0;accumulator=0;}});

window.addEventListener('pagehide',()=>{if(loaded) engine.save();});

$('#pause').addEventListener('click',action(()=>engine.pause(!engine.snapshot().paused)));

$('#swap').addEventListener('click',action(()=>engine.click(pointer.x,pointer.y,2)));

$('#mute').addEventListener('click',()=>{

    muted=!muted;activateAudio();$('#mute').textContent=muted?'Unmute':'Mute';

    if(masterGain) masterGain.gain.value=muted?0:sfxVolume;

    if(music) music.volume=muted?0:musicVolume;

});

$('#fullscreen').addEventListener('click',()=>{if(document.fullscreenElement) document.exitFullscreen();else $('#player').requestFullscreen().catch(()=>{});});
document.addEventListener('fullscreenchange',()=>{engine?.wasm.web_window(Number(Boolean(document.fullscreenElement)));});

async function load() {

    const [manifest,musicCues,wasmBytes,fontDefinitions,sources]=await Promise.all([fetch('assets.json').then(r=>r.json()),fetch('music-cues.json').then(r=>r.json()),fetch('game.wasm').then(r=>{if(!r.ok)throw new Error('Missing game module');return r.arrayBuffer();}),fetch('font-metrics.json').then(r=>r.json()),fetch('asset-sources.json').then(r=>{if(!r.ok)throw new Error('Missing asset sources');return r.json();})]);

    assets=createAssetLoader(sources);

    $('#load-message').textContent='Preparing music…';
    cues=await prepareMusic(await assets.read('content/music/zuma.mo3'),musicCues);
    $('#load-message').textContent='Loading…';

    engine=await createEngine(wasmBytes,host);

    const bitmapPaths=new Set(manifest.images.flatMap(image=>image.parts.flatMap(part=>[part.file,...(part.mask?[part.mask]:[])])));

    bitmapPaths.add('content/images/menu.png');

    for(const definition of Object.values(fontDefinitions)) for(const layer of definition.layers) bitmapPaths.add(layer.image);

    $('#progress').max=bitmapPaths.size+manifest.files.length;

    let completed=0;

    await Promise.all([...bitmapPaths].map(async path=>{const bitmap=await loadBitmap(path);bitmaps.set(path,bitmap);$('#progress').value=++completed;}));

    for(const definition of manifest.images) {

        const id=surface(definition.width,definition.height);const item=surfaces.get(id);item.name=definition.name;

        if(definition.fill) {item.context.fillStyle=definition.fill;item.context.fillRect(0,0,definition.width,definition.height);}

        for(const part of definition.parts) {

            let bitmap=bitmaps.get(part.file),source=part.source;

            if(part.mask || part.colorKey) {

                const temporary=document.createElement('canvas');temporary.width=part.cropBeforeMask?source[2]:bitmap.naturalWidth;temporary.height=part.cropBeforeMask?source[3]:bitmap.naturalHeight;

                const temporaryContext=temporary.getContext('2d',{willReadFrequently:true});
                if(part.cropBeforeMask)temporaryContext.drawImage(bitmap,...source,0,0,temporary.width,temporary.height);else temporaryContext.drawImage(bitmap,0,0);

                const data=temporaryContext.getImageData(0,0,temporary.width,temporary.height);

                if(part.mask) {

                    temporaryContext.clearRect(0,0,temporary.width,temporary.height);temporaryContext.drawImage(bitmaps.get(part.mask),0,0);

                    const mask=temporaryContext.getImageData(0,0,temporary.width,temporary.height);

                    for(let i=0;i<data.data.length;i+=4) {data.data[i+3]=part.invertMask?255-mask.data[i]:mask.data[i];if(part.maskOnly)data.data[i]=data.data[i+1]=data.data[i+2]=255;}

                } else for(let i=0;i<data.data.length;i+=4) if(part.colorKey.every((value,channel)=>data.data[i+channel]===value))data.data[i+3]=0;

                temporaryContext.putImageData(data,0,0);bitmap=temporary;if(part.cropBeforeMask)source=[0,0,temporary.width,temporary.height];

            }

            item.context.drawImage(bitmap,...source,...(part.dest || [0,0,definition.width,definition.height]));

        }

        engine.registerImage(definition,id);

    }

    const fontSurfaces=new Map();let nextFont=1;

    for(const [resource,font] of Object.entries(fontDefinitions)) {

        for(const layer of font.layers) {

            if(!fontSurfaces.has(layer.image)) {

                const bitmap=bitmaps.get(layer.image);

                const id=surface(bitmap.naturalWidth,bitmap.naturalHeight);

                surfaces.get(id).context.drawImage(bitmap,0,0,bitmap.naturalWidth,bitmap.naturalHeight);

                fontSurfaces.set(layer.image,id);

            }

            layer.surface=fontSurfaces.get(layer.image);

        }

        const id=nextFont++;fonts.set(id,font);engine.registerFont(resource,id,font);

    }

    for(const [resource,path] of Object.entries(manifest.sounds)) sounds.set(engine.soundId(resource),path);

    await Promise.all(manifest.files.map(async path=>{engine.registerFile(path,await assets.read(path));$('#progress').value=++completed;}));

    try {

        for(let i=0;i<localStorage.length;++i) {

            const key=localStorage.key(i);if(key?.startsWith(namespace)) {const value=localStorage.getItem(key);engine.registerFile(key.slice(namespace.length),Uint8Array.from(atob(value),char=>char.charCodeAt(0)));}

        }

    } catch {}

    engine.init(params.has('qa')?123456789:Date.now()>>>0);

    levels=engine.levels();loaded=true;

    $('#loading').hidden=true;

    if(params.has('qa')) window.zuma={engine,render,snapshot:()=>engine.snapshot(),balls:()=>engine.balls(),levels,drawIssues:()=>[...drawIssues.values()],pauseLoop:()=>{loaded=false;},surfaces,fonts,fontDefinitions,audioState:()=>({context:audioContext?.state,buffers:audioBuffers.size,voices:activeSounds.size,music:music?.src})};

    render();requestAnimationFrame(frame);

}

load().catch(fail);

