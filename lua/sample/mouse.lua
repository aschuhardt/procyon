position = { x = 0, y = 0 }

pr.window.on_draw = function()
  pr.draw.string(100, 100, string.format("Position: (%d, %d)", position.x, position.y))
  pr.draw.string(100, 200, "Click to change pr.colors")
end

pr.input.on_mouse_moved = function(x, y)
  position.x = math.floor(x)
  position.y = math.floor(y)
end

pr.input.on_mouse_released = function(button)
  if button.value == pr.MOUSE_LEFT then
    pr.window.set_color(pr.color.from_rgb(0.8, 0.3, 0.3))
  else
    pr.window.set_color(pr.color.from_rgb(0.1, 0.4, 0.8))
  end
end

pr.input.on_mouse_pressed = function(button)
  pr.window.set_color(pr.color.from_rgb(0.4, 0.6, 0.5))
end
