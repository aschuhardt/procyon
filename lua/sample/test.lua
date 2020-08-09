other = require("other")

local y = 0
local x = 0

local width = 0
local height = 0

local delta = 5

local input_text = "\x04\x02\x03 >"

local line_height = 0

local function on_key_pressed(key, shift, ctrl, alt) 
  local n = delta
  if shift then
    n = n + (delta * 2)
  end

  if key == keys.KEY_F2 then
    window.reload()
  end
  
  if key == keys.KEY_ESCAPE then
    window.close()
  elseif key == keys.KEY_R then
    other.do_thing()
  elseif key == keys.KEY_UP then
    y = y - n 
  elseif key == keys.KEY_DOWN then
    y = y + n 
  elseif key == keys.KEY_LEFT then 
    x = x - n 
  elseif key == keys.KEY_RIGHT then
    x = x + n
  end

end

local function on_char_entered(c)
  input_text = input_text..c
end

local function on_draw()
  draw.string(x, y, input_text, color.from_rgb(0.3, 0.2, 0.78), color.from_rgb(0.8, 0.7, 0.1))
  draw.string(x, y + line_height, input_text)
  draw.rect(100, 100, 64, 64, color.from_rgb(0.6, 0.4, 0.0))
  draw.line(400, 400, 100, 100, color.from_rgb(1.0, 1.0, 0.0))
  
  for n = 3, 8 do
    script.run(string.format("log.info(\"n: %d\")", n))
    draw.poly(200, 320, 90.2, n)
  end
end

local function on_load()
  local w, h = window.glyph_size()
  line_height = h
  log.info(string.format("Glyph size: %dx%d", w, h))
  window.set_color(color.from_rgb(0.01, 0.3, 0.45));
end

local function on_unload()
  log.info("Goodbye!")
end

input.on_key_released = nil
input.on_key_pressed = on_key_pressed 
input.on_char_entered = on_char_entered

window.on_draw = on_draw
window.on_load = on_load
window.on_unload = on_unload
