#version 330

uniform sampler2D u_GlyphTexture;

in vec2 f_TexCoords;

void main(void) {
  vec3 value = texture(u_GlyphTexture, f_TexCoords).rrr;
  gl_FragColor = vec4(value, 1.0);
  // gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
