#version 330

uniform sampler2D u_GlyphTexture;

in vec2 f_TexCoords;
in vec3 f_Color;

void main(void) {
  vec3 value = texture(u_GlyphTexture, f_TexCoords).rrr;
  gl_FragColor = vec4(value * f_Color, 1.0);
}
