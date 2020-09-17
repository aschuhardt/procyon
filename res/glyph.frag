#version 330

uniform sampler2DArray u_GlyphTexture;

in vec2 f_TexCoords;
in vec3 f_ForeColor;
in vec3 f_BackColor;
in float f_Bold;

void main(void) {
  float value = floor(texture(u_GlyphTexture, vec3(f_TexCoords, f_Bold)).r);
  gl_FragColor = vec4(mix(f_BackColor, f_ForeColor, value), 1.0);
}
