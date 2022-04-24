pr.window.on_load = function()
  initial = pr.plane.from(11, 11, function(x, y) return (x * 2) % y end)

  local encoded = initial:encode()
  pr.log.info("Encoded plane data: "..encoded)

  decoded = pr.plane.decode(encoded)
end

pr.window.on_draw = function()
  -- draw initial
  pr.draw.string(20, 6, "initial plane:")
  initial:foreach(
    function(x, y, value)
      local n = tonumber(value + 1) / 10.0
      local color = pr.color.from_rgb(n, n, n)
      pr.draw.string(x * 18 + 20, y * 18 + 20, tostring(value), color)
    end)
    
  -- draw decoded
  pr.draw.string(300, 6, "decoded plane:")
  decoded:foreach(
    function(x, y, value)
      local n = tonumber(value + 1) / 10.0
      local color = pr.color.from_rgb(n, n, n)
      pr.draw.string(x * 18 + 300, y * 18 + 20, tostring(value), color)
    end)
end
