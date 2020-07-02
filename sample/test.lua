other = require("other")

window.refresh()

local y = 0
local x = 0

local function key_pressed(key)
  if keys[key] ~= nil then
    log.info(keys[key].name)
  end
  
  if key == keys.KEY_ESCAPE then
    window.close()
  elseif key == keys.KEY_R then
    other.do_thing()
  elseif key == keys.KEY_UP then
    y = y - 1
  elseif key == keys.KEY_DOWN then
    y = y + 1
  elseif key == keys.KEY_LEFT then 
    x = x - 1
  elseif key == keys.KEY_RIGHT then
    x = x + 1
  end

  draw.string(x, y, "AAAAA")
  draw.string(x + 1, y + 3, "AAAAA")
  log.debug(string.format("%d, %d", x, y))
end

input.key_released = nil
input.key_pressed = key_pressed; 
