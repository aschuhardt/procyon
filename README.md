# Procyon Engine
A lightweight game engine written in C.

Goals:
- Provide a straightforward, batteries-included toolkit for building roguelike games
- Implement an intuitive, event-based scripting API
- Facilitate simple distribution and (non-existent) asset management by bundling everything into a single executable binary file
- Target OpenGL 3.3 for broad device support

TODO:
- [x] Creating a window
- [x] Tightly-coupled Lua integration
- [x] Event-based input handling (keyboard only for now)
- [x] Compile-time asset embedding (currently done in CMakeLists.txt)
- [ ] More flexible non-cmake asset pipeline
- [x] Bitmap font rendering
- [ ] Colored text
- [ ] Graphics primitives
- [ ] API calls for generating continuous 1D/2D/3D noise (via stb_perlin.h)

## Usage
The engine is run via the `procyon` executable.

By default, the engine will attempt to load a file called `script/main.lua`.  This, along with several other settings, can be overridden via command-line options:

```
Usage: procyon [--(log level)] [-e|--entry (file path)]
Log levels: error (default), warn, info, debug, trace
A GPU-accelerated tile-based game engine with scripting via Lua


General
    -h, --help            show this help message and exit

Logging
    --error               enable ERROR log level
    --warn                enable WARN log level
    --info                enable INFO log level
    --debug               enable DEBUG log level
    --trace               enable TRACE log level

Script loading
    -e, --entry=<str>     script entry point (default = 'script/main.lua')

Visuals
    -w, --width=<int>     window width
    -h, --height=<int>    window height
    -s, --scale=<flt>     text scale (default = 1.0)
```

The scripting API itself is described below.

## Modules
Procyon uses Lua for scripting.  High-level objects are grouped into global tables that I refer to as "modules".  Modules consist of various pieces of functionality bearing similar areas of concern.

### Window
#### Functions
- `window.close()` - Returns nothing.  Closes the window.
- `window.size()` - Returns two integers (width and height), representing the dimensions of the window in pixels.
- `window.glyph_size()` - Returns two integers (width and height), representing the *scaled* dimensions of text glyphs.

#### Fields
- `window.on_draw`
- `window.on_resize`
- `window.on_load`
- `window.on_unload`

### Drawing
#### Functions
- `draw.string(x, y, contents)`

### Input
#### Fields
- `input.on_key_pressed`
- `input.on_key_released`
- `input.on_char_entered`
- `keys.KEY_A ...`
- `keys[k].name`
- `keys[k].value`

### Utility
#### Functions
- `log.info(contents)`
- `log.error(contents)`
- `log.warn(contents)`
- `log.debug(contents)`

