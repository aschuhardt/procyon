window.on_load = function()
  text_buffer = plane.from(100, 100, string.byte("."))
  text = ""
  offset = 0.1
  window.set_high_fps(true)
end

window.on_draw = function(seconds)
  offset = offset + seconds * 0.05
  text_buffer:fill(function(x, y, c)
    local value = math.floor((noise.ridge(tonumber(x) * 0.1, tonumber(y) * 0.1, offset) + 0.5) * 6 + 41)
    return value
  end)

  local gw, gh = window.glyph_size()
  for y = 0, text_buffer.height - 1 do
    for x = 0, text_buffer.width - 1 do
      draw.char(x * gh, y * gh, text_buffer:at(x, y))
    end
  end
  draw.string(0, 0, "%b%iFPS: "..tostring(math.floor(1.0 / seconds)))
end

input.on_char_entered = function(c)
  text = text..c
  text_buffer:set(0, 0, text)
end



input.on_key_pressed = function(key, shift, ctrl, alt) 
  if key == KEY_F2 then
    window.reload()
  elseif key == KEY_ESCAPE then
    window.close()
  end
end
