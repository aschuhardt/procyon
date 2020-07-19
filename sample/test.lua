other = require("other")

local y = 0
local x = 0

local width = 0
local height = 0

local delta = 5

local input_text = ">"

local function on_key_pressed(key)
  -- if keys[key] ~= nil then
  --  log.info(keys[key].name)
  -- end
  
  if key == keys.KEY_ESCAPE then
    window.close()
  elseif key == keys.KEY_R then
    other.do_thing()
  elseif key == keys.KEY_UP then
    y = y - delta 
  elseif key == keys.KEY_DOWN then
    y = y + delta
  elseif key == keys.KEY_LEFT then 
    x = x - delta
  elseif key == keys.KEY_RIGHT then
    x = x + delta
  end
end

local function on_char_entered(c)
  input_text = input_text..c
end

local function on_draw()
  draw.string(x, y, input_text)
  draw.string(x, y + 36, input_text)
end

local function on_load()
  log.info("Hello!")
  local w, h = window.glyph_size()
  log.info(string.format("Glyph size: %dx%d", w, h))
end

local function on_unload()
  log.info("Goodbye!")
end

input.on_key_released = nil
input.on_key_pressed = key_pressed 
input.char_entered = char_entered

window.on_draw = on_draw
window.on_load = on_load
window.on_unload = on_unload
