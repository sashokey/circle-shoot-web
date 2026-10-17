function mapped(font, character) {
    return font.mapping[character] ?? character;
}

export function measureFont(font, value) {
    let width = 0;
    let previous = '';
    for (const character of value) {
        const current = mapped(font, character);
        width += Math.max(0, ...font.layers.map(layer => (layer.chars[current]?.width ?? 0) + (previous ? layer.spacing + (layer.kerning[previous + current] ?? 0) : 0)));
        previous = current;
    }
    return width;
}

export function fontCommands(font, value, x, baseline, rgba) {
    const commands = [];
    let advance = 0;
    const characters = [...value].map(character => mapped(font, character));
    for (let i = 0; i < characters.length; ++i) {
        let width = 0;
        for (const layer of font.layers) {
            const glyph = layer.chars[characters[i]];
            const spacing = i + 1 < characters.length ? layer.spacing + (layer.kerning[characters[i] + characters[i + 1]] ?? 0) : 0;
            width = Math.max(width, (glyph?.width ?? 0) + spacing);
            if (!glyph?.rect || !glyph.rect[2] || !glyph.rect[3]) continue;
            const offset = glyph.offset ?? [0, 0];
            const channels = [rgba.red, rgba.green, rgba.blue, Math.round(rgba.alpha * 255)].map((channel, index) => Math.min(255, Math.trunc(channel * Math.trunc(layer.colorMult[index] * 255) / 255) + Math.trunc(layer.colorAdd[index] * 255)));
            commands.push({
                surface: layer.surface,
                rect: glyph.rect,
                x: x + advance + layer.offset[0] + offset[0],
                y: baseline - layer.ascent + layer.offset[1] + offset[1],
                color: (channels[3] << 24) | (channels[0] << 16) | (channels[1] << 8) | channels[2],
                order: layer.order
            });
        }
        advance += width;
    }
    commands.sort((a, b) => a.order - b.order);
    return commands;
}
