padding = 1
render_size = 10
color_inactive = pr.color.from_rgb(0.2, 0.1, 0.8)
select_position = { x = 0, y = 0 }

function color_from_value(val)
  if val ~= 0 then
    return pr.color.from_rgb(0.2, tonumber(val) / 255.0, 0.2)
  else
    return color_inactive
  end
end

function render_plane_at(x, y, plane)
  plane:foreach(
    function(px, py, val)
      pr.draw.rect(
        x + px * (render_size + padding),
        y + py * (render_size + padding),
        render_size,
        render_size,
        color_from_value(val))
    end)
end

function update()
  result = pr.plane.from_wfc(result.width, result.height, tile, 
    true, true, 1, tile_size, tile_size)
end

pr.window.on_load = function()
  tile = pr.plane.from(8, 8, 0)
  tile_size = tile.width / 4

  result = pr.plane.from(64, 64, 0)
end

pr.window.on_draw = function()
  render_plane_at(5, 5, tile)
  render_plane_at(5, 200, result)

  pr.draw.string(200, 5, string.format("tile size: %d", tile_size))
end

pr.input.on_mouse_moved = function(x, y)
  select_position.x = x
  select_position.y = y
end

pr.input.on_mouse_pressed = function(button)
  local x, y = select_position.x, select_position.y
  if x > 5 
    and x <= 5 + tile.width * (render_size + padding)
    and y > 5
    and y <= 5 + tile.height * (render_size + padding) then

    local tx = math.floor((x - 5) / (render_size + padding))
    local ty = math.floor((y - 5) / (render_size + padding))

    local val = tile:at(tx, ty)
    tile:set(tx, ty, val + 10)

    update()
  end
end


pr.input.on_key_pressed = function(key)
  if key.value == pr.KEY_ESCAPE then
    pr.window.close()
  elseif key.value == pr.KEY_UP then
    tile_size = tile_size + 1
    update()
  elseif key.value == pr.KEY_DOWN then
    tile_size = tile_size - 1
    update()
  end
end
