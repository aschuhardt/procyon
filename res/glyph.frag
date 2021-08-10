#version 330

uniform sampler2DArray u_GlyphTexture;

in vec2 f_TexCoords;
flat in int f_ForeColor;
flat in int f_BackColor;
in float f_Bold;

void main(void) {
  vec3 fg = vec3(
      ((f_ForeColor & 0xFF0000) >> 16) / 256.0,
      ((f_ForeColor & 0xFF00) >> 8) / 256.0,
      ((f_ForeColor & 0xFF)) / 256.0);
  vec3 bg = vec3(
      ((f_BackColor & 0xFF0000) >> 16) / 255.0,
      ((f_BackColor & 0xFF00) >> 8) / 255.0,
      ((f_BackColor & 0xFF)) / 255.0);

  float value = floor(texture(u_GlyphTexture, vec3(f_TexCoords, f_Bold)).r);
  gl_FragColor = vec4(mix(bg, fg, value), 1.0);
}
