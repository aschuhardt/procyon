local util = {}

-- converts HTML-style hexadecimal color codes to an engine-friendly format
function util.hex_to_color(hex)
  hex = hex:gsub("#", "")

  return color.from_rgb(
    tonumber("0x" .. hex:sub(1, 2)) / 255.0, 
    tonumber("0x" .. hex:sub(3, 4)) / 255.0, 
    tonumber("0x" .. hex:sub(5, 6)) / 255.0)
end


-- returns a function that will return a normalized noise value with the
-- provided parameters based on an X and Y value
function util.make_scaled_noise(scale, offset)
  return function(x, y)
    local noise_value = noise.fbm(tonumber(x) * scale, tonumber(y) * scale, offset)
    local normalized = (noise_value + 1.0) / 2.0
    return math.floor(normalized * 255.0)
  end
end

return util
