#version 330

uniform sampler2D u_FrameTexture;

in vec2 f_TexCoords;

void main(void) {
  // gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
  gl_FragColor = texture(u_FrameTexture, f_TexCoords);
}
