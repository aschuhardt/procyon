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

  for i = 0, 15
  do
    draw.string(x + i * 20, y + i * 20, "\x01 Hello! \x02")
  end 

  if key == keys.KEY_U then
    window.font_size = 64
  end
end

local function char_entered(c)
  log.info("Character entered: %s", c)
end

input.key_released = nil
input.key_pressed = key_pressed 
input.char_entered = char_entered
