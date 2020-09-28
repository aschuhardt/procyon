#ifndef SHADER_GLYPH_H
#define SHADER_GLYPH_H

#include "shader.h"

typedef struct procy_glyph_bounds_t procy_glyph_bounds_t;

typedef struct procy_glyph_shader_program_t {
  procy_shader_program_t program;
  unsigned int u_ortho, u_sampler, font_texture;
  int texture_w, texture_h;
  size_t glyph_count;
  float glyph_scale;
  void *index_buffer, *vertex_buffer;
} procy_glyph_shader_program_t;

/*
 * Builds and compiles a shader program with information for rendering text
 * glyphs from a bitmap font.
 */
procy_glyph_shader_program_t* procy_create_glyph_shader(float scale);

/*
 * Builds and executes a draw call on the GPU, consisting of vertex data built
 * from all of the `GLYPH` type draw operations
 */
void procy_draw_glyph_shader(procy_glyph_shader_program_t* shader,
                             struct procy_window_t* window);

/*
<<<<<<< HEAD
 * Computes scaled glyph bounds in pixels
 */
void procy_get_glyph_bounds(procy_glyph_shader_program_t* shader, int* width,
                            int* height);

/*
 * Disposes of a glyph shader program and deletes its bound resources from
 * the OpenGL context
=======
 * Disposes of a glyph shader program and deletes its bound resources from the
 * OpenGL context
>>>>>>> 5a47373a6b26524ae1a6b8c48f72e9a3f7a16059
 */
void procy_destroy_glyph_shader(procy_glyph_shader_program_t* shader);

#endif
