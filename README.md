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
- `window.on_draw` - If assigned, `on_draw` is called before each new frame is drawn.  Perform any drawing routines here.  No arguments are passed to `on_draw`.
- `window.on_resize` - If assigned, `on_resize` is called when the window is resized.  Two arguments, `width` and `height`, are passed to the function. 
- `window.on_load` - If assigned, `on_load` is called prior to the beginning of the main game loop.  Perform any initialization here.  No arguments are passed to `on_load`.
- `window.on_unload` - If assigned, `on_unload` is called after the main game loop has terminated.  Perform any cleanup logic here.  No arguments are passed to `on_unload`.

---

### Drawing

#### Functions
- `draw.string(x, y, contents)` - Draws `contents` at pixel coordinates `(x, y)` on the window.

---

### Input

#### Fields
- `input.on_key_pressed` - If assigned, `on_key_pressed` is called when a key is pressed.  It is passed a single argument, `key`, which is one of the `keys.KEY_...` fields.  `key` has members `name` and `value`, though they are not required to be used.  See the example implementation provided below:

```lua
local function handle_key_pressed(key)
  -- key info can be indexed by the key object itself
  log.debug("Key %s was pressed!", keys[key].name)

  if key == keys.KEY_ESCAPE then
    window.close()
  end
end

window.on_key_pressed = handle_key_pressed
```

- `input.on_key_released` - Same as `on_key_pressed` above, but called when a key is released.
- `input.on_char_entered` - If assigned, `on_char_entered` is called when the user enters text via the keyboard.
- `keys.KEY_A ...` - Named key objects.  See `script/keys.h` for a complete list.
- `keys[k].name`, `keys[k].value` - In addition to named objects, data pertaining to each key is stored in the `keys` table indexed by the key values themselves.

---

### Utility

#### Functions
- `log.info(contents)`
- `log.error(contents)`
- `log.warn(contents)`
- `log.debug(contents)`

