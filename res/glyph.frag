#version 330

uniform sampler2D u_GlyphTexture;

in vec2 f_TexCoords;
flat in uint f_Color; // RGBA

void main(void) {
  uint mask = uint(255);
  uint red_shift = uint(24);
  uint green_shift = uint(16);
  uint blue_shift = uint(8);

  vec4 color = vec4(float(f_Color >> red_shift   & mask),
                    float(f_Color >> green_shift & mask),
                    float(f_Color >> blue_shift  & mask),
                    float(f_Color                & mask));
  color /= 255.0;

  vec4 value = texture(u_GlyphTexture, f_TexCoords).rrrr;
  gl_FragColor = vec4(value * color);
}
