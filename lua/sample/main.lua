local util = require "util"


-- Perform any initialization logic inside of window.load.
--
-- At this point all of the engine's resources will be available
-- for the developer to use.
window.on_load = function()

  if jit.status() then
    log.debug("JIT is enabled!")
  else
    log.debug("JIT is disabled!")
  end

  -- build a couple of 2D bitmaps filled with continuous noise values
  -- for our somewhat natural-looking map generation
  color_map  = plane.from(80, 80, util.make_scaled_noise(0.1,   1.2))
  sprite_map = plane.from(80, 80, util.make_scaled_noise(0.3,  -10.0))
  tree_map   = plane.from(80, 80, util.make_scaled_noise(0.08, -0.3))
    :foreach(util.minimum(128))
  ocean_map  = plane.from(80, 80, util.make_scaled_noise(0.15,  7.1))

  -- load a spritesheet from a PNG file
  sheet = spritesheet.load("sprites.png")

  -- load some sprites from the spritesheet
  tree_sprites = {
    sheet:sprite(80, 16, 16, 16),
    sheet:sprite(64, 16, 16, 16),
    sheet:sprite(48, 16, 16, 16),
    sheet:sprite(32, 16, 16, 16),
    sheet:sprite(16, 16, 16, 16),
    sheet:sprite( 0, 16, 16, 16),
    sheet:sprite(64, 32, 16, 16),
    sheet:sprite(48, 32, 16, 16),
    sheet:sprite(32, 32, 16, 16),
    sheet:sprite(16, 32, 16, 16),
    sheet:sprite( 0, 32, 16, 16),
  }

  -- specify some colors to use for drawing sprites
  red = color.from_rgb(1.0, 0.0, 0.0)
  yellow = color.from_rgb(0.85, 0.85, 0.0)
  tree_background = util.hex_to_color("#313638")
  water_colors = {
    base = util.hex_to_color("#0b5e65"),
    deep = util.hex_to_color("#0b8a8f")
  }
  tree_colors = {
    util.hex_to_color("#676633"),
    util.hex_to_color("#165a4c"),
    util.hex_to_color("#547e64"),
    util.hex_to_color("#374e4a"),
    util.hex_to_color("#239063")
  }
end


-- All rendering logic should be implemented in window.on_draw.
--
-- This is called once per frame.
window.on_draw = function(seconds)
  draw.set_layer(1)

  local ww, wh = window.size()
  local gw, gh = window.glyph_size()

  draw.string(0, 10,
    "%bA%b: scale-up; %bZ%b: scale-down; %bF11%b: toggle fullscreen; %bF2%b: reload; %bESC%b: quit")

  if high_fps then
    draw.string(0, 30,
      "%bF3%b: toggle high-fps mode (currently %benabled%b)", red);
    draw.string(0, wh - gh, string.format("%%iFPS: %.2f", 1.0 / seconds))
  else
    draw.string(0, 30,
      "%bF3%b: toggle high-fps mode (currently %bdisabled%b)", yellow);
  end

  draw.string(0, 60, "click a sprite to highlight it")

  draw.set_layer(2) -- draw behind the text (higher layer values are further back)

  -- iterate over each value in the bitmap, drawing tree sprites on most of them 
  tree_map:foreach(
    function(x, y, value)
      if value > 0 then
        local tree_sprite = pick_sprite(x, y)
        local tree_color = pick_color(x, y)
        local bg_color = tree_background

        -- sprite size, position
        local sw, sh = tree_sprite:get_size();
        local sx = x * sw + ww / 2 - sw * tree_map.width  / 2
        local sy = y * sh + wh / 2 - sh * tree_map.height / 2

        -- if the mouse cursor is intersecting with this sprite, color it red
        if cursor_position and 
           cursor_position.x >= sx and 
           cursor_position.x <  sx + sw and
           cursor_position.y >= sy and 
           cursor_position.y <  sy + sh then
         hovered = { x = x, y = y }
         tree_color = red
        end

        if selection and selection.x == x and selection.y == y then
          tree_color = yellow
        end

        tree_sprite:draw(sx, sy, tree_color, bg_color)
      end
    end)
end


-- Input events, if defined, are called in response to various
-- kinds of user-input.

input.on_mouse_moved = function(x, y)
  cursor_position = { x = x, y = y }
end

input.on_mouse_pressed = function(button)
  if hovered and button.value == MOUSE_LEFT then
    selection = { x = hovered.x, y = hovered.y }
  end
end


input.on_key_pressed = function(key) 
  if key.value == KEY_F2 then
    window.reload()
  elseif key.value == KEY_F3 then
    high_fps = not high_fps
    window.set_high_fps(high_fps)
  elseif key.value == KEY_ESCAPE then
    window.close()
  elseif key.value == KEY_Z then
    local x, y = window.get_scale()
    window.set_scale(x - 0.1, y - 0.1)
  elseif key.value == KEY_A then
    local x, y = window.get_scale()
    window.set_scale(x + 0.1, y + 0.1)
  elseif key.value == KEY_F11 then
    fullscreen = not fullscreen -- doesn't need to exist beforehand; Lua treats null values as falsey
    if fullscreen then
      window.set_fullscreen()
    else
      window.set_windowed()
    end
  end
end


function pick_sprite(x, y)
  local value = sprite_map:at(x, y) / 255.0
  return tree_sprites[math.floor(value * tonumber(#tree_sprites)) + 1]
end


function pick_color(x, y)
  local value = color_map:at(x, y) / 255.0
  return tree_colors[math.floor(value * tonumber(#tree_colors)) + 1]
end
