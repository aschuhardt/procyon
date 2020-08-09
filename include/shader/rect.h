#ifndef SHADER_RECT_H
#define SHADER_RECT_H

#include "shader.h"

typedef struct procy_rect_shader_program_t {
  unsigned int u_ortho;
  procy_shader_program_t program;
} procy_rect_shader_program_t;

procy_rect_shader_program_t procy_create_rect_shader();

void procy_destroy_rect_shader_program(procy_rect_shader_program_t* shader);

void procy_draw_rect_shader(procy_rect_shader_program_t* shader,
                            struct procy_window_t* window,
                            struct procy_draw_op_t* ops, size_t n);

#endif
