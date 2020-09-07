local current_text = "Enter some text: "

window.on_draw = function(seconds)
  local gw, gh = window.glyph_size()
  local w, h = window.size()

  draw.string(4, 4, "Press F2 to reload, or Escape to exit")
  draw.string(4, 4 + gh, string.format("Window size: %dx%d", w, h))

  draw.string(100, 100, current_text, color.from_rgb(0.7, 0.8, 0.4))
  draw.line(100, 100 + gh, 100 + #current_text * gw, 100 + gh)

  draw.rect(200, 130, 60, 40, color.from_rgb(0, 0, 1))
  draw.rect(200, 130, 30, 20, color.from_rgb(1, 0, 0))
  draw.rect(230, 150, 30, 20, color.from_rgb(1, 0, 0))

  draw.poly(200, 170, 20, 7) 

  -- add a sixth argument to indicate whether text is drawn vertically
  draw.string(100, 100 + gh * 2, "This text is vertical!",
    color.from_rgb(0.5, 0.1, 1.0), color.from_rgb(0, 0, 0), true)
end

input.on_char_entered = function(c)
  current_text = current_text..c
end

input.on_key_pressed = function(key, shift, ctrl, alt) 
  if key == KEY_F2 then
    window.reload()
  elseif key == KEY_ESCAPE then
    window.close()
  end
end
