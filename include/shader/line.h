#ifndef SHADER_line_H
#define SHADER_line_H

#include "shader.h"

typedef struct procy_line_shader_program_t {
  unsigned int u_ortho;
  procy_shader_program_t program;
  void* vertex_buffer;
  size_t line_count;
} procy_line_shader_program_t;

procy_line_shader_program_t procy_create_line_shader();

void procy_destroy_line_shader(procy_line_shader_program_t* shader);

void procy_draw_line_shader(procy_line_shader_program_t* shader,
                            struct procy_window_t* window);

#endif
