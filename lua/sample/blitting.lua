pr.window.on_load = function()
  base_plane = pr.plane.from(64, 64, 20)

  stamp_plane = pr.plane.from(
    {0,   50, 0},
    {50,  0,  50},
    {200, 0,  200})

  --[[
  stamp_plane = pr.plane.from(3, 3, 80)
  stamp_plane:set(1, 0, 50)
  stamp_plane:set(0, 1, 50)
  stamp_plane:set(2, 1, 50)
  stamp_plane:set(1, 2, 50)
  ]]
end

pr.window.on_draw = function()
  pr.draw.set_layer(1)
  pr.draw.string(0, 10, 'Click on the grid to blit a pattern from another grid onto it')

  pr.draw.set_layer(2)
  base_plane:foreach(
    function(x, y, value)
      local color_val = tonumber(value) / 255.0
      pr.draw.rect(x * 10, y * 10, 8, 8, 
        pr.color.from_rgb(color_val, color_val, color_val))
    end)
end

pr.input.on_mouse_moved = function(x, y)
  mouse_x = x
  mouse_y = y
end

pr.input.on_mouse_pressed = function()
  local offset_x = mouse_x / 10
  local offset_y = mouse_y / 10

  if offset_x >= 0 
    and offset_x < base_plane.width
    and offset_y >= 0
    and offset_y < base_plane.height then
    base_plane:blit(offset_x, offset_y, stamp_plane)
  end
end
