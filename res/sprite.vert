#version 330

uniform mat4 u_Ortho;

layout(location = 0) in vec3 i_Position;
layout(location = 1) in vec2 i_TexCoords;
layout(location = 2) in int i_ForeColor;
layout(location = 3) in int i_BackColor;

out vec2 f_TexCoords;
flat out int f_ForeColor;
flat out int f_BackColor;
out float f_Depth;

void main(void) {
  f_TexCoords = i_TexCoords;
  f_ForeColor = i_ForeColor;
  f_BackColor = i_BackColor;
  f_Depth = i_Position.z * 0.1;
  gl_Position = vec4(i_Position.xy, 0.0, 1.0) * u_Ortho;
}
