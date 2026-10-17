export function prepareMusic(bytes, cues) {
    return new Promise((resolve, reject) => {
        const worker = new Worker(new URL('./music-worker.js', import.meta.url));
        const result = {};
        const urls = [];
        const fail = error => {
            worker.terminate();
            for (const url of urls) URL.revokeObjectURL(url);
            reject(error);
        };
        worker.onerror = event => fail(new Error(event.message || 'Unable to prepare music'));
        worker.onmessageerror = () => fail(new Error('Unable to receive prepared music'));
        worker.onmessage = ({data}) => {
            if (data.error) return fail(new Error(data.error));
            if (data.done) {
                if (Object.keys(result).length !== Object.keys(cues).length) return fail(new Error('Incomplete music cues'));
                worker.terminate();
                resolve(result);
            } else {
                const file = URL.createObjectURL(new Blob([data.wav], {type: 'audio/wav'}));
                urls.push(file);
                result[data.order] = {...cues[data.order], file};
            }
        };
        worker.postMessage({bytes, cues});
    });
}
