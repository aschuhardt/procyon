#ifndef SHADER_H
#define SHADER_H

#include <stdbool.h>
#include <stddef.h>

typedef struct shader_program_t {
  unsigned int vertex, fragment, program, vao, *vbo;
  size_t vbo_count;
  bool valid;
} shader_program_t;

struct window_t;
struct glyph_t;

/*
 * Builds and compiles a shader program with information for rendering text
 * glyphs from a bitmap font. Note that this does not allocate its object on the
 * heap.
 *
 * After calling, check the `valid` flag on the resulting object in order to
 * determine whether the program successfully compiled.
 */
shader_program_t create_glyph_shader();

void draw_glyphs(shader_program_t* program, struct window_t* window,
                 struct glyph_t* data, size_t count);

/*
 * Calls the OpenGL commands required to un-bind and free up GPU resources
 * associated with the shader program.
 */
void destroy_shader_program(shader_program_t* program);

#endif
