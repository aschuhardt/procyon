#ifndef SHADER_H
#define SHADER_H

#include <stdbool.h>
#include <stddef.h>

struct procy_window_t;
struct procy_draw_op_t;

typedef struct procy_shader_program_t {
  unsigned int vertex, fragment, program, vao, *vbo;
  size_t vbo_count;
  bool valid;
} procy_shader_program_t;

typedef struct procy_glyph_shader_program_t {
  unsigned int u_ortho, u_sampler, font_texture;
  int texture_w, texture_h;
  float scale;
  struct procy_window_t* window;
  procy_shader_program_t program;
} procy_glyph_shader_program_t;

/*
 * Builds and compiles a shader program with information for rendering text
 * glyphs from a bitmap font. Note that this does not allocate its object on the
 * heap.
 */
procy_glyph_shader_program_t procy_create_glyph_shader(
    struct procy_window_t* window);

/*
 * Builds and executes a draw call on the GPU, consisting of vertex data built
 * from all of the `GLYPH` type draw operations
 */
void procy_draw_glyph_shader(procy_glyph_shader_program_t* shader,
                             struct procy_window_t* window,
                             struct procy_draw_op_t* ops, size_t n);

/*
 * Disposes of a glyph shader program and deletes its bound resources from the
 * OpenGL context
 */
void procy_destroy_glyph_shader_program(procy_glyph_shader_program_t* shader);

#endif
