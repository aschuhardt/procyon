#ifndef WINDOW_H
#define WINDOW_H

#include <stdbool.h>
#include <stdlib.h>

#include "color.h"

struct procy_draw_op_t;
struct procy_key_info_t;
struct procy_state_t;
struct procy_glyph_shader_program_t;
struct procy_rect_shader_program_t;
struct procy_line_shader_program_t;
struct GLFWwindow;

typedef struct procy_draw_op_buffer_t {
  struct procy_draw_op_t* buffer;
  size_t length;
  size_t capacity;
} procy_draw_op_buffer_t;

typedef struct procy_shaders_t {
  struct procy_glyph_shader_program_t* glyph;
  struct procy_rect_shader_program_t* rect;
  struct procy_line_shader_program_t* line;
} procy_shaders_t;

typedef struct procy_window_t {
  procy_draw_op_buffer_t draw_ops;
  procy_shaders_t shaders;
  float ortho[4][4];
  bool quitting, high_fps;
  struct procy_state_t* state;
  struct procy_key_info_t* key_table;
  struct GLFWwindow* glfw_win;
} procy_window_t;

procy_window_t* procy_create_window(int width, int height, const char* title,
                                    float text_scale,
                                    struct procy_state_t* state);

void procy_destroy_window(procy_window_t* window);

void procy_begin_loop(procy_window_t* window);

void procy_append_draw_op(procy_window_t* window,
                          struct procy_draw_op_t* draw_op);

void procy_append_draw_ops(procy_window_t* window,
                           struct procy_draw_op_t* draw_ops, size_t n);

void procy_get_window_size(procy_window_t* window, int* width, int* height);

void procy_get_glyph_size(procy_window_t* window, int* width, int* height);

void procy_set_clear_color(procy_color_t c);

void procy_set_glyph_scale(procy_window_t* window, float scale);

void procy_close_window(procy_window_t* window);

#endif
