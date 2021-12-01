#version 330

uniform mat4 u_Ortho;

layout(location = 0) in vec3 i_Position;
layout(location = 1) in int i_Color;

flat out int f_Color;
out float f_Depth;

void main(void) {
  f_Color = i_Color;
  f_Depth = i_Position.z * 0.1;
  gl_Position = vec4(i_Position.xy, 0.0, 1.0) * u_Ortho;
}
