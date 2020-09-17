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

void procy_destroy_shader_program(procy_shader_program_t* shader);

bool procy_compile_frag_shader(const char* data, unsigned int* index);

bool procy_compile_vert_shader(const char* data, unsigned int* index);

bool procy_link_shader_program(unsigned int vert, unsigned int frag,
                               unsigned int* index);

#endif
