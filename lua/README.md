This is a wrapper around `libprocyon` that adds Lua scripting and some other useful features.

## Usage

The engine is run via the `procyon-lua` executable.

By default, the engine will attempt to load a file called `script/main.lua`.  This, along with several other settings, can be overridden via command-line options:

```
Usage: procyon [--(log level)] [[-e|--entry (file path)] | (file path)]
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

Methods and constants are grouped into tables within a global 'pr' table.

### Window

#### Functions

- `pr.window.close()` - Returns nothing.  Closes the window.
- `pr.window.get_size()` - Returns two integers (width and height), representing the dimensions of the window in pixels.
- `pr.window.get_glyph_size()` - Returns two integers (width and height), representing the *scaled* dimensions of text glyphs.
- `pr.window.get_scale()` - Returns a floating-point number representing the window's current visual scale.  The default is 1.0.
- `pr.window.set_scale(scale)` - Returns nothing.  Sets the scale of the visuals presented in the window (controlled via orthographic projection matrix).
- `pr.window.reset_scale()` - Returns nothing.  Sets the scale of the visuals presented in the window to the default value, which is 1.0.
- `pr.window.reload()` - Returns nothing.  Causes the window and script environment to be disposed-of and reloaded (functionally "restarts" the app).
- `pr.window.set_color(color)` - Returns nothing.  Sets the "clear" color used for the window background.
- `pr.window.set_high_fps(enabled)` - Returns nothing.  Enables or enables "high-fps mode", in which the window will attempt to update with a frequency that matches the display refresh rate.  Otherwise, when high-fps mode is disabled, the window only updates every full second  or when input events are triggered.
- `pr.window.set_fullscreen()` - Returns nothing.  Switches from windowed mode to fullscreen.
- `pr.window.set_windowed()` - Returns nothing.  Switches from fullscreen mode to windowed.

#### Fields
- `pr.window.on_draw` - If assigned, `on_draw` is called before each new frame is drawn.  Perform any drawing routines here.
  A single floating-point argument `seconds` is passed to `on_draw`.  It represents the amount of time, in fractional seconds, since the last frame was drawn.
- `pr.window.on_resize` - If assigned, `on_resize` is called when the window is resized.  Two arguments, `width` and `height`, are passed to the function. 
- `pr.window.on_load` - If assigned, `on_load` is called prior to the beginning of the main game loop.  Perform any initialization here.  No arguments are passed to `on_load`.
- `pr.window.on_unload` - If assigned, `on_unload` is called after the main game loop has terminated.  Perform any cleanup logic here.  No arguments are passed to `on_unload`.

---

### Drawing

#### Functions

- `pr.draw.set_layer(z)` - Returns nothing.  Sets the current drawing layer to the provided integer value, which should be greater than zero.  Higher values of `z` are further-back relative to the window, with a value of 1 being the effective "top layer" that will always be visible.
- `pr.draw.string(x, y, contents [, color [, background]])` - Draws `contents` at screen coordinates `(x, y)` on the window. 
- `pr.draw.rect(x, y, width, height [, color])` - Draws a rectangle with the provided integer bounds and optional color.
- `pr.draw.line(x1, y1, x2, y2 [, color])` - Draws a line from the pixel coordinates `(x1, y1)` to `(x2, y2)`.
- `pr.draw.poly(x, y, radius, n [, color])` - Draws an `n`-sided polygon centered at pixel coordinates `(x, y)`, with a floating point `radius`, and an optional color.
- `pr.color.from_rgb(r, g, b)` - Returns a table with fields `r`, `g`, `b`, and `a` that represents a color value.  Arguments should be floating-point values between `0.0` and `1.0`.
- `pr.spritesheet.load(path)` - Return a new spritesheet object built from an image file at `path`.  Note that there is a cap on the number of spritesheets that can be loaded during the lifetime of the application (currently 32).  Modify `MAX_SPRITE_SHADER_COUNT` in `window.h` if you need to raise this cap for some reason.
- `pr.spritesheet.load(table)` - Returns a new spritesheet object build from raw data found in a binary buffer.  The argument should be a table with two fields: `length`, which is an integer, and `buffer`, which is a lightuserdata that contains raw texture data.  `length` should describe the length, in bytes, of `buffer`.
- `spritesheet:sprite(x, y, w, h)` - Returns a new sprite object defined by the provided position and dimensions within the spritesheet's texture.  The table that is returned has its `width` and `height` fields set accordingly.
- `sprite:draw(x, y, [, color [, background]])` - Draws the sprite at the provided screen coordinates.

---

### Input

#### Fields

- `pr.input.on_key_pressed` - If assigned, `on_key_pressed` is called when a key is pressed. It is passed a single argument, a table with the following fields:
    - `value` - An integer indicating the scancode of the key that was entered.  Comparable to the `KEY_...` global values.
    - `ctrl`, `alt`, `shift` - Boolean flags indicating whether one of these modifiers was pressed at the time the event was raised.
```lua
pr.input.on_key_pressed = function(key)
  if key.value == pr.KEY_ESCAPE then
    pr.log.info(string.format('Goodbye!'))
    pr.window.close()
  end
end
```

- `pr.input.on_key_released` - Same as `on_key_pressed` above, but called when a key is released.
- `pr.input.on_char_entered` - If assigned, `on_char_entered` is called when the user enters text via the keyboard.
- `pr.KEY_A ...` - Named key objects.  See `script/keys.h` for a complete list.
- `pr.input.on_mouse_pressed` - If assigned, `on_mouse_pressed is called when a mouse button is pressed.  It is passed a single argument, a table with the following fields:
  - `value` - An integer representing the button that was pressed.  One of `pr.MOUSE_LEFT`, `pr.MOUSE_RIGHT`, `pr.MOUSE_MIDDLE`, or `pr.MOUSE_[0,8]`.
  - `ctrl`, `alt`, `shift` - Boolean flags indicating whether one of these modifiers was pressed at the time the event was raised.
- `pr.input.on_mouse_released` - Same as `on_mouse_pressed` above, but called when the button is released.
- `pr.input.on_mouse_moved` - If assigned, `on_mouse_moved` is called when the mouse has moved.  It is passed two floating-point arguments, `x` and `y`, which represent the new position in screen coordinates of the mouse cursor.

---

### Noise

Continuous noise is useful for all kinds of things.

#### Functions

- `pr.noise.perlin(x, y, z [, seed])` - Returns a floating-point value.  Only the lower 8 bits of `seed` are used.  Refer to `stb_perlin.h` for more information.
- `pr.noise.ridge(x, y, z [, lacunarity, gain, offset, octaves])` - Returns a floating-point value.  Refer to `stb_perlin.h` for more information.
- `pr.noise.fbm(x, y, z [, lacunarity, gain, octaves])` - Returns a floating-point value.  Refer to `stb_perlin.h` for more information.
- `pr.noise.turbulence(x, y, z [, lacunarity, gain, octaves])` - Returns a floating-point value.  Refer to `stb_perlin.h` for more information.

---

### Plane

A plane is a 2D bitmap data structure.  Elements in the plane consist of 32-bit signed integer values, and are accessed by an (X, Y) index.  Planes have a fixed `width` and `height` which are exposed to the developer.

#### Functions

- Creation
  - `pr.plane.from(w, h, value)` - Returns a new plane object with dimensions `w` and `h`, having each of its elements initialized to `value`.  The dimensions are made accessible via the `width` and `height` fields in the resulting table.
  - `pr.plane.from(w, h, function(x, y, cur))` - Returns a new plane object with dimensions `w` and `h`, having each of its elements initialized to the value returned by the function `func`, which is passed arguments `x, y` corresponding to the index being initialized.  `func` is also passed a third value, `cur`, which is always zero and can be ignored.
  - `plane:sub(x, y, w, h)` - Returns a new plane with dimensions `w` and `h`, having its values copied from the plane on which this method is called starting at position `(X, Y)`.  In other words, this returns a copied region from within the target.
  - `pr.plane.from_wfc(w, h, (path|tile_plane), [flipx], [flipy], [nrot], [tilew], [tileh])` - Returns a new plane object with dimensions `w` and `h`, with its contents being the result of running [the WafeFunctionCollapse algorithm](https://github.com/mxgmn/WaveFunctionCollapse) to completion based on the provided input plane (either an image on-disk or another plane; for the former see `import`).  The WFC algorithm works by breaking the source plane up into a number of smaller tiles.  The behavior of the algorithm can be customized by specifying whether or not to flip each tile on the X or Y axes (booleans `flipx` and `flipy`), the maximum number of tile rotations to perform (`nrot`), and the dimensions of each tile (`tilew` and `tileh`, default is 3).
- Reading
  - `plane:at(x, y)` - Returns the value of the element at the index `(X, Y)`.
  - `plane:foreach(function(x, y, cur))` - An alias for `plane:fill` intended to be passed a function that doesn't return anything.  See `plane:fill` below.
  - `plane:find_all(function(x, y, cur))` - The function argument is evaluated against each cell in the plane, and may return either a true or false value.  Returns a list of tables in the form { x, y, value } corresponding to the cells for which the filter function returned a true value.
  - `plane:find_first(function(x, y, cur))` - The function argument is evaluated against each cell in the plane until a true value is returned, and may return either a true or false value.  Returns a table in the form { x, y, value } corresponding to the cell for which the filter function returned a true value.
- Serialization
  - `plane:export(path)` - Exports the plane to a PNG image.  Only the lower three bytes of each element in the plane are exported, with the highest byte being mapped to the alpha channel and being set to 255.  The pixel format is (A)RGB.
  - `pr.plane.import(path)` - Imports a plane from a PNG image.  See `export`.
  - `plane:encode()` - Returns a base-64 string consisting of the plane data, packed using the very fast [SIMDComp library](https://github.com/lemire/simdcomp).
  - `pr.plane.decode(encoded_str)` - Returns a plane decoded from a base-64 string as created by `plane:encode()`.  See `encode` above.
- Modification
  - `plane:set(x, y, n)` - Sets the value of the element in the plane at index `(X, Y)` to `n`.
  - `plane:fill(n)` - Returns a reference to the plane. Sets the value of each element in the plane to `n`.
  - `plane:fill(function(x, y, cur))` - Returns a reference to the plane. Sets the value of each element in the plane to the return value of the provided function, to which is passed the current position as well as the current value of each element in the plane.
  - `plane:copy()` - Returns a copy of the plane.
  - `plane:blit(x, y, src)` - Copies or 'blits' the contents of the plane `src` onto the target plane at position `(X, Y)`. Useful for 'stamping' a pre-defined pattern onto a plane, for example.

---

### Utility

#### Functions

- `pr.script.run(contents)` - Returns nothing.  Runs the script value in the global environment.  Use caution when executing arbitrary code.  TODO: create a sandboxed environment for this.
- `pr.util.popcount(value)` - Returns the number of '1' bits in the integer parameter.
- `pr.log.info(contents)`
- `pr.log.error(contents)`
- `pr.log.warn(contents)`
- `pr.log.debug(contents)`
