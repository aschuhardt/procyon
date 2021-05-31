This is a wrapper around `libprocyon` that adds Lua scripting. 

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
- `window.on_char_entered` - If assigned, `on_char_entered` is called when the user types text input.  It is passed a single argument: a string containing the character that was entered.
- `window.on_key_pressed` - If assigned, `on_key_pressed` is called when a key has been pressed.  It is passed one argument consisting of a table with the following fields:
  * `value` - An integer value indicating the key that was pressed.  The available key values are stored globally by name, with values accessible by their names, such as `KEY_A`.  See `include/keys.h` for all available values.
  * `name` - A string containing the name of the key.  This is the same name as is used for the key values stored globally.
  * `shift` - A boolean value indicating whether the Shift modifier was pressed.
  * `ctrl` - A boolean value indicating whehter the Ctrl modifier was pressed.
  * `alt` - A boolean value indicating whether the Alt modifier was pressed.
- `window.on_key_released` - See previous.

---

### Drawing

#### Functions
- `draw.string(x, y, contents [, forecolor [, backcolor]])` - Draws `contents` at pixel coordinates `(x, y)` on the window. 
- `draw.rect(x, y, width, height [, color])` - Draws a rectangle with the provided integer bounds and optional color.
- `draw.line(x1, y1, x2, y2 [, color])` - Draws a line from the pixel (integer) coordinates `(x1, y1)` to `(x2, y2)`.
- `draw.poly(x, y, radius, n [, color])` - Draws an `n`-sided polygon centered at pixel (integer) coordinates `(x, y)`, with a floating point `radius`, and an optional color.
- `color.from_rgb(r, g, b)` - Returns a table with fields `r`, `g`, `b`, and `a` that represents a color value.  Arguments should be floating-point values between `0.0` and `1.0`.

---

### Input

#### Fields
- `input.on_key_pressed` - If assigned, `on_key_pressed` is called when a key is pressed.  It has four arguments: `key`, which is one of the `KEY_...` global values, followed by and  three boolean values indicating whether the `shift`, `control`, or `alt` mods are in effect.  See the example implementation provided below:

```lua
input.on_key_pressed = function(key, shift, ctrl, alt)
  if key == KEY_ESCAPE then
    window.close()
  end
end
```

- `input.on_key_released` - Same as `on_key_pressed` above, but called when a key is released.
- `input.on_char_entered` - If assigned, `on_char_entered` is called when the user enters text via the keyboard.
- `KEY_A ...` - Named key objects.  See `script/keys.h` for a complete list.
- `keys[k].name`, `keys[k].value` - In addition to named objects, data pertaining to each key is stored in the `keys` table indexed by the key values themselves.

---

### Utility

#### Functions
- `noise.perlin(x, y, z [, seed])` - Returns a floating-point value.  Only the lower 8 bits of `seed` are used.  Refer to `stb_perlin.h` for more information.
- `noise.ridge(x, y, z [, lacunarity, gain, offset, octaves])` - Returns a floating-point value.  Refer to `stb_perlin.h` for more information.
- `noise.fbm(x, y, z [, lacunarity, gain, octaves])` - Returns a floating-point value.  Refer to `stb_perlin.h` for more information.
- `noise.turbulence(x, y, z [, lacunarity, gain, octaves])` - Returns a floating-point value.  Refer to `stb_perlin.h` for more information.
- `script.run(contents)` - Returns nothing.  Runs the script value in the global environment.  Use caution when executing arbitrary code.
- `log.info(contents)`
- `log.error(contents)`
- `log.warn(contents)`
- `log.debug(contents)`

