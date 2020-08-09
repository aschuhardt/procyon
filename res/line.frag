#version 330

in vec3 f_Color;

void main(void) {
  gl_FragColor = vec4(f_Color, 1.0);
}
