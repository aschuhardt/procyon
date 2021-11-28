#include "shader/frame.h"

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "gen/frame_frag.h"
#include "gen/frame_vert.h"
#include "shader/error.h"
#include "window.h"

#define VBO_FRAME_VERTICES 0
#define VBO_FRAME_INDICES 1
#define ATTR_FRAME_POSITION 0
#define ATTR_FRAME_TEXCOORDS 1

typedef procy_frame_shader_program_t frame_shader_program_t;
typedef procy_shader_program_t shader_program_t;
typedef procy_window_t window_t;

// clang-format off

const static float FRAME_QUAD_VERTICES[] = {
  // position    // texture
  -1.0F,  1.0F,  0.0F, 1.0F,    // top-left     
   1.0F,  1.0F,  1.0F, 1.0F,    // top-right
  -1.0F, -1.0F,  0.0F, 0.0F,    // bottom-left
   1.0F, -1.0F,  1.0F, 0.0F     // bottom-right
};

const static unsigned short FRAME_QUAD_INDICES[] = {
  // first triangle
  //    0--1
  //    | /
  //    2/
  0, 1, 2, 

  // second triangle
  //      /1
  //     / |
  //    2--3
  1, 3, 2
};

// clang-format on

procy_frame_shader_program_t *procy_create_frame_shader(window_t *window) {
  frame_shader_program_t *shader = calloc(1, sizeof(frame_shader_program_t));

  // generate + bind framebuffer
  GL_CHECK(glGenFramebuffers(1, &shader->framebuffer));
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, shader->framebuffer));

  // generate + bind framebuffer texture
  GL_CHECK(glGenTextures(1, &shader->texture));
  GL_CHECK(glBindTexture(GL_TEXTURE_2D, shader->texture));

  // set texture parameters and unbind
  int width;
  int height;
  procy_get_window_size(window, &width, &height);
  GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                        GL_UNSIGNED_BYTE, NULL));
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
  GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

  // attach the texture to the framebuffer and unbind
  GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_TEXTURE_2D, shader->texture, 0));
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // time to set up shader program stuff...
  shader_program_t *program = &shader->program;

  // generate VAO
  glGenVertexArrays(1, &program->vao);

  // generate VBOs
  program->vbo_count = 2;
  program->vbo = malloc(sizeof(GLuint) * program->vbo_count);
  glGenBuffers(program->vbo_count, program->vbo);

  procy_compile_and_link_shader(program, (char *)&embed_frame_vert[0],
                                (char *)&embed_frame_frag[0]);

  return shader;
}

void procy_destroy_frame_shader(frame_shader_program_t *shader) {
  procy_destroy_shader_program(&shader->program);

  if (glIsTexture(shader->texture)) {
    glDeleteTextures(1, &shader->texture);
  }

  if (glIsFramebuffer(shader->framebuffer)) {
    glDeleteFramebuffers(1, &shader->framebuffer);
  }

  free(shader);
}

void procy_frame_shader_begin(frame_shader_program_t *shader) {
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, shader->framebuffer));
  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
}

void procy_frame_shader_end(frame_shader_program_t *shader) {
  GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
}

void procy_frame_shader_resized(frame_shader_program_t *shader, int width,
                                int height) {
  // regenerate the framebuffer texture at the correct size
  GL_CHECK(glBindTexture(GL_TEXTURE_2D, shader->texture));
  GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                        GL_UNSIGNED_BYTE, NULL));
  GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
}

void procy_draw_frame_shader(frame_shader_program_t *shader) {
  shader_program_t *program = &shader->program;

  GL_CHECK(glUseProgram(program->program));
  GL_CHECK(glBindVertexArray(program->vao));

  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, program->vbo[VBO_FRAME_VERTICES]));
  // position
  GL_CHECK(glEnableVertexAttribArray(ATTR_FRAME_POSITION));
  GL_CHECK(glVertexAttribPointer(ATTR_FRAME_POSITION, 2, GL_FLOAT, GL_FALSE,
                                 sizeof(float) * 4, 0));

  // texture coords
  GL_CHECK(glEnableVertexAttribArray(ATTR_FRAME_TEXCOORDS));
  GL_CHECK(glVertexAttribPointer(ATTR_FRAME_TEXCOORDS, 2, GL_FLOAT, GL_FALSE,
                                 sizeof(float) * 4,
                                 (void *)(sizeof(float) * 2)));  // NOLINT

  // copy vertices
  GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(FRAME_QUAD_VERTICES),
                        &FRAME_QUAD_VERTICES[0], GL_STATIC_DRAW));

  // copy indices
  GL_CHECK(
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, program->vbo[VBO_FRAME_INDICES]));
  GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(FRAME_QUAD_INDICES),
                        &FRAME_QUAD_INDICES, GL_STATIC_DRAW));

  // draw to the screen
  GL_CHECK(glBindTexture(GL_TEXTURE_2D, shader->texture));
  GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
  GL_CHECK(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0));

  // clean up
  GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
  GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
  GL_CHECK(glBindVertexArray(0));
  GL_CHECK(glUseProgram(0));
}

