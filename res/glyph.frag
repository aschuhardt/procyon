#version 330

uniform sampler2D u_GlyphTexture;

in vec2 f_TexCoords;
in vec3 f_ForeColor;
in vec3 f_BackColor;

void main(void) {
  float value = texture(u_GlyphTexture, f_TexCoords).r;
  if (value == 0.0) {
    gl_FragColor = vec4(f_BackColor, 1.0);
  } else {
    gl_FragColor = vec4(f_ForeColor, 1.0);
  }
}

