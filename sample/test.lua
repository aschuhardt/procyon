other = require("other")

local y = 0
local x = 0

local width = 0
local height = 0

local delta = 5;

local function key_pressed(key)
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

  draw.string(x, y, string.format("Tile width: %d x %d", width, height))
  draw.string(x + 20, y + 20, string.format("Tile width: %d x %d", width, height))
  draw.string(x + 30, y + 10, string.format("Tile width: %d x %d", width, height))
end

input.key_released = nil
input.key_pressed = key_pressed; 
