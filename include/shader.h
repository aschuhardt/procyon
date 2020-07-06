#ifndef SHADER_H
#define SHADER_H

#include <stdbool.h>
#include <stddef.h>

typedef struct shader_program_t {
  unsigned int vertex, fragment, program, vao, *vbo;
  size_t vbo_count;
  bool valid;
} shader_program_t;

typedef struct glyph_shader_program_t {
  shader_program_t program;
  unsigned int u_ortho, u_sampler, font_texture;
  void* codepoints;
} glyph_shader_program_t;

struct window_t;
struct glyph_t;

/*
 * Builds and compiles a shader program with information for rendering text
 * glyphs from a bitmap font. Note that this does not allocate its object on the
 * heap.
 */
glyph_shader_program_t create_glyph_shader();

void draw_glyphs(glyph_shader_program_t* glyph_shader, struct window_t* window,
                 struct glyph_t* data, size_t count);

void destroy_glyph_shader_program(glyph_shader_program_t* program);

#endif
