#version 330

flat in int f_Color;

void main(void) {
  vec3 color = vec3(
      ((f_Color & 0xFF0000) >> 16) / 255.0,
      ((f_Color & 0xFF00) >> 8) / 255.0,
      (f_Color & 0xFF) / 255.0);
  gl_FragColor = vec4(color, 1.0);
}
