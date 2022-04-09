pr.window.on_load = function()
  text_buffer = pr.plane.from(100, 100, string.byte("."))
  text = ""
  offset = 0.1
  pr.window.set_high_fps(true)
end

pr.window.on_draw = function(seconds)
  offset = offset + seconds * 0.05
  text_buffer:fill(function(x, y, c)
    local value = math.floor((pr.noise.ridge(tonumber(x) * 0.1, tonumber(y) * 0.1, offset) + 0.5) * 6 + 41)
    return value
  end)

  local gw, gh = pr.window.glyph_size()
  for y = 0, text_buffer.height - 1 do
    for x = 0, text_buffer.width - 1 do
      pr.draw.char(x * gh, y * gh, text_buffer:at(x, y))
    end
  end
  pr.draw.string(0, 0, "%b%iFPS: "..tostring(math.floor(1.0 / seconds)))
end

pr.input.on_char_entered = function(c)
  text = text..c
  text_buffer:set(0, 0, text)
end

pr.input.on_key_pressed = function(key) 
  if key.value == pr.KEY_F2 then
    pr.window.reload()
  elseif key.value == pr.KEY_ESCAPE then
    pr.window.close()
  end
end
