#ifndef SHADER_GLYPH_H
#define SHADER_GLYPH_H

#include "shader.h"

struct procy_draw_op_text_t;

typedef struct procy_glyph_shader_program_t {
  procy_shader_program_t program;
  void *vertex_batch_buffer, *index_batch_buffer;
  unsigned int u_ortho, u_sampler, font_texture;
  struct {
    int width, height;
  } texture_bounds;
  struct {
    int width, height;
    float tex_width, tex_height;
  } glyph_bounds;
} procy_glyph_shader_program_t;

/*
 * Builds and compiles a shader program with information for rendering text
 * glyphs from a bitmap font.
 */
procy_glyph_shader_program_t *procy_create_glyph_shader();

/*
 * Builds and executes a draw call on the GPU, consisting of vertex data built
 * from all of the `GLYPH` type draw operations
 */
void procy_draw_glyph_shader(procy_glyph_shader_program_t *shader,
                             struct procy_window_t *window,
                             struct procy_draw_op_text_t *draw_ops);

/*
 * Computes glyph bounds in pixels
 */
void procy_get_glyph_bounds(procy_glyph_shader_program_t *shader, int *width,
                            int *height);

/*
 * Disposes of a glyph shader program and deletes its bound resources from
 * the OpenGL context
 */
void procy_destroy_glyph_shader(procy_glyph_shader_program_t *shader);

#endif
