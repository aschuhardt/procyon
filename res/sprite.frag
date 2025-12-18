#version 330

uniform sampler2D u_SpriteTexture;

in vec2 f_TexCoords;
flat in int f_ForeColor;
flat in int f_BackColor;
in float f_Depth;

void main(void) {
  vec3 fg = vec3(
      ((f_ForeColor & 0xFF0000) >> 16) / 255.0,
      ((f_ForeColor & 0xFF00) >> 8) / 255.0,
      ((f_ForeColor & 0xFF)) / 255.0);
  vec3 bg = vec3(
      ((f_BackColor & 0xFF0000) >> 16) / 255.0,
      ((f_BackColor & 0xFF00) >> 8) / 255.0,
      ((f_BackColor & 0xFF)) / 255.0);

  vec3 black = vec3(0.0, 0.0, 0.0);
  vec4 color = texture(u_SpriteTexture, f_TexCoords);
  if (color.rgb == black) {
    if (bg != black) {
      // color is black and we have a background specified
      gl_FragColor = vec4(bg, color.a);
    } else {
      // color is black and there is no background - fully transparent
      gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    }
  } else {
    gl_FragColor = vec4(color.rgb * fg, color.a);
  }

  gl_FragDepth = f_Depth;
}
