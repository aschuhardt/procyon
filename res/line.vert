#version 330

uniform mat4 u_Ortho;

layout(location = 0) in vec2 i_Position;
layout(location = 1) in vec3 i_Color;

out vec3 f_Color;

void main(void) {
  f_Color = i_Color;
  gl_Position = vec4(i_Position, 0.0, 1.0) * u_Ortho;
}
