#version 330

uniform vec2 u_WindowSize;
uniform vec2 u_TileSize;

layout(location = 0) in vec2 i_Position;

void main(void) {
  vec2 normalized = (i_Position / u_WindowSize - 1.0);
  vec2 y_reversed = normalized * vec2(1.0, -1.0);
  vec2 scaled = y_reversed * u_TileSize;
  gl_Position = vec4(scaled, 0.0, 1.0);
}
