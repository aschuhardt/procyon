#ifndef WINDOW_H
#define WINDOW_H

#include <stdbool.h>
#include <stdlib.h>

#include "color.h"

struct procy_draw_op_t;
struct procy_key_info_t;
struct procy_state_t;
struct GLFWwindow;

typedef struct procy_window_bounds_t {
  int width, height;
} procy_window_bounds_t;

typedef struct procy_glyph_bounds_t {
  int width, height;
} procy_glyph_bounds_t;

typedef struct procy_draw_op_buffer_t {
  struct procy_draw_op_t* buffer;
  size_t length;
  size_t capacity;
} procy_draw_op_buffer_t;

typedef struct procy_window_t {
  procy_draw_op_buffer_t draw_ops;
  procy_window_bounds_t bounds;
  procy_glyph_bounds_t glyph;
  struct procy_state_t* state;
  struct procy_key_info_t* key_table;
  float ortho[4][4], text_scale;
  struct GLFWwindow* glfw_win;
  bool quitting, high_fps;
} procy_window_t;

procy_window_t* procy_create_window(int width, int height, const char* title,
                                    float text_scale,
                                    struct procy_state_t* state);

void procy_destroy_window(procy_window_t* window);

void procy_begin_loop(procy_window_t* window);

void procy_append_draw_op(procy_window_t* window,
                          struct procy_draw_op_t* draw_op);

void procy_set_clear_color(procy_color_t c);

void procy_close_window(procy_window_t* window);

#endif
