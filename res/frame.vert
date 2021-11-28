#version 330

layout(location = 0) in vec2 i_Position;
layout(location = 1) in vec2 i_TexCoords;

out vec2 f_TexCoords;

void main(void) {
  f_TexCoords = i_TexCoords;
  gl_Position = vec4(i_Position, 0.0, 1.0);
}
