#version 330

uniform mat4 u_Ortho;

layout(location = 0) in vec2 i_Position;
layout(location = 1) in vec2 i_TexCoords;
layout(location = 2) in vec3 i_ForeColor;
layout(location = 3) in vec3 i_BackColor;

out vec2 f_TexCoords;
out vec3 f_ForeColor;
out vec3 f_BackColor;

void main(void) {
  f_TexCoords = i_TexCoords;
  f_ForeColor = i_ForeColor;
  f_BackColor = i_BackColor;
  gl_Position = vec4(i_Position, 0.0, 1.0) * u_Ortho;
}
