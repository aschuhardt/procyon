local current_text = "Enter some text: "

window.on_load = function()
  map = plane.from(80, 80,
    function(x, y)
      local noise_value = noise.perlin(tonumber(x) * 0.1, tonumber(y) * 0.1, 1.2)
      local normalized = (noise_value + 1.0) / 2.0
      return math.floor(normalized * 255.0)
    end)

  sheet = spritesheet.load("sprites.png")

  if sheet then
    tree_sprite = sheet:sprite(80, 16, 16, 16)
  end

  window.set_high_fps(true)

  fps = 0
end

function draw_map()
  local start_x = 200
  local start_y = 200
  local size = 4
  for x = 0, map.width do
    for y = 0, map.height do
      local val = map:at(x, y) / 255.0
      draw.rect(start_x + x * size, start_y + y * size, size, size, color.from_rgb(val, val, val))
    end
  end
end

window.on_draw = function(seconds)
  local gw, gh = window.glyph_size()
  local w, h = window.size()

  fps = 1.0 / seconds

  draw.string(0, h - gh, "%iFPS: " .. tostring(fps))

  if tree_sprite then
    tree_sprite:draw(10, 500);
  end

  draw.line(0, 0, 600, 600);

  draw.string(4, 4, "Press %b%iF2%i%b to %breload%b, or %b%iEscape%i%b to exit")
  draw.string(4, 4 + gh, "Here is an escaped percent-sign: %%")
  draw.string(4, 4 + gh * 2, string.format("Window size: %dx%d", w, h))

  draw.string(100, 100, current_text, color.from_rgb(0.7, 0.8, 0.4))

  draw.rect(200, 130, 60, 40, color.from_rgb(0, 0, 1))
  draw.rect(200, 130, 30, 20, color.from_rgb(1, 0, 0))
  draw.rect(230, 150, 30, 20, color.from_rgb(1, 0, 0))

  draw.poly(200, 170, 20, 7) 

  draw_map()
end

input.on_char_entered = function(c)
  current_text = current_text..c
end

input.on_key_pressed = function(key) 
  if key.value == KEY_F2 then
    window.reload()
  elseif key.value == KEY_ESCAPE then
    window.close()
  elseif key.value == KEY_Z then
    local scale = window.get_scale()
    window.set_scale(scale - 0.25)
  elseif key.value == KEY_A then
    local scale = window.get_scale()
    window.set_scale(scale + 0.25)
  end
end
