#version 330

uniform sampler2D u_GlyphTexture;

in vec2 f_TexCoords;
in vec4 f_Color;

void main(void) {
  vec4 value = texture(u_GlyphTexture, f_TexCoords).rrrr;
  gl_FragColor = vec4(value * f_Color);
}
