This is a wrapper around `libprocyon` that adds Lua scripting and some other useful features.

## Usage
The engine is run via the `procyon-lua` executable.

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
```

The scripting API itself is described below.

## Modules

High-level objects are grouped into global tables that I refer to as "modules".  Modules consist of various pieces of functionality with similar areas of concern.

### Window

#### Functions
- `window.close()` - Returns nothing.  Closes the window.
- `window.size()` - Returns two integers (width and height), representing the dimensions of the window in pixels.
- `window.glyph_size()` - Returns two integers (width and height), representing the *scaled* dimensions of text glyphs.
- `window.get_scale()` - Returns a floating-point number representing the window's current visual scale.  The default is 1.0.
- `window.set_scale(scale)` - Returns nothing.  Sets the scale of the visuals presented in the window (controlled via orthographic projection matrix).
- `window.reset_scale()` - Returns nothing.  Sets the scale of the visuals presented in the window to the default value, which is 1.0.
- `window.reload()` - Returns nothing.  Causes the window and script environment to be disposed-of and reloaded (functionally "restarts" the app).
- `window.set_color(color)` - Returns nothing.  Sets the "clear" color used for the window background.
- `window.set_high_fps(enabled)` - Returns nothing.  Enables or enables "high-fps mode", in which the window will attempt to update with a frequency that matches the display refresh rate.  Otherwise, when high-fps mode is disabled, the window only updates every full second  or when input events are triggered.

#### Fields
- `window.on_draw` - If assigned, `on_draw` is called before each new frame is drawn.  Perform any drawing routines here.
  A single floating-point argument `seconds` is passed to `on_draw`.  It represents the amount of time, in fractional seconds, since the last frame was drawn.
- `window.on_resize` - If assigned, `on_resize` is called when the window is resized.  Two arguments, `width` and `height`, are passed to the function. 
- `window.on_load` - If assigned, `on_load` is called prior to the beginning of the main game loop.  Perform any initialization here.  No arguments are passed to `on_load`.
- `window.on_unload` - If assigned, `on_unload` is called after the main game loop has terminated.  Perform any cleanup logic here.  No arguments are passed to `on_unload`.

---

### Drawing

#### Functions
- `draw.string(x, y, contents [, color [, background]])` - Draws `contents` at screen coordinates `(x, y)` on the window. 
- `draw.rect(x, y, width, height [, color])` - Draws a rectangle with the provided integer bounds and optional color.
- `draw.line(x1, y1, x2, y2 [, color])` - Draws a line from the pixel coordinates `(x1, y1)` to `(x2, y2)`.
- `draw.poly(x, y, radius, n [, color])` - Draws an `n`-sided polygon centered at pixel coordinates `(x, y)`, with a floating point `radius`, and an optional color.
- `color.from_rgb(r, g, b)` - Returns a table with fields `r`, `g`, `b`, and `a` that represents a color value.  Arguments should be floating-point values between `0.0` and `1.0`.
- `spritesheet.load(path)` - Return a new spritesheet object built from an image file at `path`.  Note that there is a cap on the number of spritesheets that can be loaded during the lifetime of the application (currently 32).  Modify `MAX_SPRITE_SHADER_COUNT` in `window.h` if you need to raise this cap for some reason.
- `spritesheet.load(table)` - Returns a new spritesheet object build from raw data found in a binary buffer.  The argument should be a table with two fields: `length`, which is an integer, and `buffer`, which is a lightuserdata that contains raw texture data.  `length` should describe the length, in bytes, of `buffer`.
- `spritesheet:sprite(x, y, w, h)` - Returns a new sprite object defined by the provided position and dimensions within the spritesheet's texture.  The table that is returned has its `width` and `height` fields set accordingly.
- `sprite:draw(x, y, [, color [, background]])` - Draws the sprite at the provided screen coordinates.

---

### Input

#### Fields
- `input.on_key_pressed` - If assigned, `on_key_pressed` is called when a key is pressed. It is passed a single argument, a table with the following fields:
    - `value` - An integer indicating the scancode of the key that was entered.  Comparable to the `KEY_...` global values.
    - `name` - A string indicating the name of the key that was pressed.  The same name is used as the names of the `KEY_...` global values.
    - `ctrl`, `alt`, `shift` - Boolean flags indicating whether one of these modifier was pressed at the time the event was raised.
```lua
input.on_key_pressed = function(key)
  if key.value == KEY_ESCAPE then
    log.info(string.format('%s was pressed!', key.name))
    window.close()
  end
end
```

- `input.on_key_released` - Same as `on_key_pressed` above, but called when a key is released.
- `input.on_char_entered` - If assigned, `on_char_entered` is called when the user enters text via the keyboard.
- `KEY_A ...` - Named key objects.  See `script/keys.h` for a complete list.
- `keys[k].name`, `keys[k].value` - In addition to named objects, data pertaining to each key is stored in the `keys` table indexed by the key values themselves.

---

### Noise

Continuous noise is useful for all kinds of things.

#### Functions
- `noise.perlin(x, y, z [, seed])` - Returns a floating-point value.  Only the lower 8 bits of `seed` are used.  Refer to `stb_perlin.h` for more information.
- `noise.ridge(x, y, z [, lacunarity, gain, offset, octaves])` - Returns a floating-point value.  Refer to `stb_perlin.h` for more information.
- `noise.fbm(x, y, z [, lacunarity, gain, octaves])` - Returns a floating-point value.  Refer to `stb_perlin.h` for more information.
- `noise.turbulence(x, y, z [, lacunarity, gain, octaves])` - Returns a floating-point value.  Refer to `stb_perlin.h` for more information.

---

### Plane

A plane is a 2D bitmap data structure.  Elements in the plane consist of 8-bit integers, and are accessed by an (X, Y) index.  Planes have a fixed width and height which are exposed to the developer.

#### Functions
- `plane.from(w, h, value)` - Returns a new plane object with dimensions `w` and `h`, having each of its elements initialized to `value`.  The dimensions are made accessible via the `width` and `height` fields in the resulting table.
- `plane.from(w, h, function(x, y))` - Returns a new plane object with dimensions `w` and `h`, having each of its elements initialized to the value returned by the function `func`, which is passed arguments `x, y` corresponding to the index being initialized.
- `plane:at(x, y)` - Returns the value of the element at the index `(X, Y)`.
- `plane:set(x, y, n)` - Sets the value of the element in the plane at index `(X, Y)` to `n`.
- `plane:set(x, y, str)` - Sets the values of elements starting at `(X, Y)` and continuing on to `(X + 1, Y), ...` for each character in the provided ASCII string.
- `plane:fill(n)` - Sets the value of each element in the plane to `n`.
- `plane:fill(function(x, y, cur))` - Sets the value of each element in the plane to the return value of the provided function, to which is passed the current position as well as the current value of each element in the plane.
- `plane:foreach(function(x, y, cur))` - An alias for `plane:fill` intended to be passed a function that doesn't return anything.

---

### Utility

#### Functions
- `script.run(contents)` - Returns nothing.  Runs the script value in the global environment.  Use caution when executing arbitrary code.  TODO: create a sandboxed environment for this.
- `log.info(contents)`
- `log.error(contents)`
- `log.warn(contents)`
- `log.debug(contents)`
- `ROOT` - A global value set to the absolute path of the directory that contains the entry script.

