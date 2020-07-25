#version 330

uniform sampler2D u_GlyphTexture;

in vec2 f_TexCoords;
in uint f_Color; // RGBA

const uint COLOR_MASK = 256;

void main(void) {
  vec3 color = vec3(f_Color >> 24 & COLOR_MASK,
                    f_Color >> 16 & COLOR_MASK,
                    f_Color >> 8  & COLOR_MASK,
                    f_Color       & COLOR_MASK);
  vec3 value = texture(u_GlyphTexture, f_TexCoords).rrr;
  gl_FragColor = vec4(value * f_Color, 1.0);
}
