#version 330

uniform sampler2D u_GlyphTexture;

in vec2 f_TexCoords;
in vec3 f_ForeColor;
in vec3 f_BackColor;

void main(void) {
  float value = texture(u_GlyphTexture, f_TexCoords).r;
  gl_FragColor = vec4(mix(f_BackColor, f_ForeColor, value), 1.0);
}
