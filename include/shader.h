#ifndef SHADER_H
#define SHADER_H

#include <stdbool.h>
#include <stddef.h>

struct window_t;
struct draw_op_t;
struct stbtt_bakedchar;

typedef struct shader_program_t {
  unsigned int vertex, fragment, program, vao, *vbo;
  size_t vbo_count;
  bool valid;
} shader_program_t;

typedef struct glyph_shader_program_t {
  shader_program_t program;
  unsigned int u_ortho, u_sampler, font_texture;
  struct window_t* window;
  struct stbtt_bakedchar* codepoints;
} glyph_shader_program_t;

/*
 * Builds and compiles a shader program with information for rendering text
 * glyphs from a bitmap font. Note that this does not allocate its object on the
 * heap.
 */
glyph_shader_program_t create_glyph_shader(struct window_t* window);

void draw_glyph_shader(glyph_shader_program_t* shader, struct window_t* window,
                       struct draw_op_t* ops, size_t n);

void destroy_glyph_shader_program(glyph_shader_program_t* shader);

void set_glyph_shader_font_size(glyph_shader_program_t* shader);

#endif
