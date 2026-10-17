import http from 'node:http';
import { createReadStream, statSync } from 'node:fs';
import { resolve, dirname, extname, sep } from 'node:path';
import { fileURLToPath } from 'node:url';

const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const port = Number(process.env.PORT || 8080);
const basePath = process.env.BASE_PATH ? '/' + process.env.BASE_PATH.replace(/^\/+|\/+$/g, '') : '';
const mime = { '.html': 'text/html; charset=utf-8', '.js': 'text/javascript; charset=utf-8', '.css': 'text/css; charset=utf-8', '.json': 'application/json', '.wasm': 'application/wasm', '.png': 'image/png', '.gif': 'image/gif', '.jpg': 'image/jpeg', '.ogg': 'audio/ogg', '.ttf': 'font/ttf', '.svg': 'image/svg+xml' };
http.createServer((request, response) => {
    try {
        let pathname = decodeURIComponent(new URL(request.url, 'http://localhost').pathname);
        if (basePath) {
            if (!pathname.startsWith(basePath + '/')) { response.writeHead(404).end('Not found'); return; }
            pathname = pathname.slice(basePath.length);
        }
        const path = resolve(root, '.' + (pathname.endsWith('/') ? pathname + 'index.html' : pathname));
        if (path !== root && !path.startsWith(root + sep)) {
            response.writeHead(403).end();
            return;
        }
        if (!statSync(path).isFile()) throw new Error('Not a file');
        response.writeHead(200, { 'Content-Type': mime[extname(path)] || 'application/octet-stream', 'Cache-Control': 'no-cache', 'X-Content-Type-Options': 'nosniff' });
        createReadStream(path).pipe(response);
    } catch {
        response.writeHead(404).end('Not found');
    }
}).listen(port, '127.0.0.1', () => console.log(`Zuma Deluxe: http://127.0.0.1:${port}${basePath}/`));
