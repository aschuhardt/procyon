#ifndef SHADER_FRAME_H
#define SHADER_FRAME_H

#include "shader.h"

typedef struct procy_frame_shader_program_t {
  procy_shader_program_t program;
  unsigned int texture, depth, framebuffer;
} procy_frame_shader_program_t;

procy_frame_shader_program_t *procy_create_frame_shader(
    struct procy_window_t *window);

void procy_destroy_frame_shader(procy_frame_shader_program_t *shader);

void procy_frame_shader_begin(procy_frame_shader_program_t *shader);

void procy_frame_shader_end(procy_frame_shader_program_t *shader);

void procy_frame_shader_resized(procy_frame_shader_program_t *shader, int width,
                                int height);

void procy_draw_frame_shader(procy_frame_shader_program_t *shader);

#endif
