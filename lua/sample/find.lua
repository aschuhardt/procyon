local f = string.format

function is_equal_one(x, y, value)
  return value == 1
end

pr.window.on_load = function()
  local p = pr.plane.from(100, 100)

  p:set(30, 40, 1)
  p:set(50, 60, 1)
  p:set(70, 80, 1)

  local all_ones = p:find_all(is_equal_one)

  for i, cell in ipairs(all_ones) do
    pr.log.info(f('%d: (%d, %d)', i, cell.x, cell.y))
  end

  local first_one = p:find_first(is_equal_one)

  pr.log.info(f('(%d, %d)', first_one.x, first_one.y))
end
