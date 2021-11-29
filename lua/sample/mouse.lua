position = { x = 0, y = 0 }

window.on_draw = function()
  draw.string(100, 100, string.format("Position: (%d, %d)", position.x, position.y))
  draw.string(100, 200, "Click to change colors")
end

input.on_mouse_moved = function(x, y)
  position.x = math.floor(x)
  position.y = math.floor(y)
end

input.on_mouse_released = function(button)
  if button.value == MOUSE_LEFT then
    window.set_color(color.from_rgb(0.8, 0.3, 0.3))
  else
    window.set_color(color.from_rgb(0.1, 0.4, 0.8))
  end
end

input.on_mouse_pressed = function(button)
  window.set_color(color.from_rgb(0.4, 0.6, 0.5))
end
