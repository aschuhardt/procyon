#ifndef WINDOW_H
#define WINDOW_H

#include <stdbool.h>
#include <stdlib.h>

#include "color.h"
#include "drawing.h"

struct procy_key_info_t;
struct procy_state_t;
struct procy_frame_shader_program_t;
struct procy_glyph_shader_program_t;
struct procy_rect_shader_program_t;
struct procy_line_shader_program_t;
struct procy_sprite_shader_program_t;
struct procy_draw_op_text_t;
struct procy_draw_op_rect_t;
struct procy_draw_op_sprite_t;
struct procy_draw_op_line_t;
struct procy_draw_op_sprite_bucket_t;
struct GLFWwindow;

typedef struct procy_window_t {
  struct {
    struct procy_glyph_shader_program_t *glyph;
    struct procy_rect_shader_program_t *rect;
    struct procy_line_shader_program_t *line;
    struct procy_frame_shader_program_t *frame;
    struct procy_sprite_shader_program_t **sprite;
  } shaders;
  float ortho[4][4];
  float scale;
  bool quitting, high_fps;
  struct procy_draw_op_text_t *draw_ops_text;
  struct procy_draw_op_rect_t *draw_ops_rect;
  struct procy_draw_op_line_t *draw_ops_line;
  struct procy_draw_op_sprite_bucket_t *draw_ops_sprite;
  struct procy_state_t *state;
  struct procy_key_info_t *key_table;
  struct GLFWwindow *glfw_win;
} procy_window_t;

procy_window_t *procy_create_window(int width, int height, const char *title,
                                    struct procy_state_t *state);

void procy_destroy_window(procy_window_t *window);

void procy_begin_loop(procy_window_t *window);

void procy_append_draw_op_text(procy_window_t *window,
                               struct procy_draw_op_text_t *op);

void procy_append_draw_op_rect(procy_window_t *window,
                               struct procy_draw_op_rect_t *op);

void procy_append_draw_op_sprite(procy_window_t *window,
                                 struct procy_draw_op_sprite_t *op);

void procy_append_draw_op_line(procy_window_t *window,
                               struct procy_draw_op_line_t *op);

void procy_append_sprite_shader(procy_window_t *window,
                                struct procy_sprite_shader_program_t *shader);

void procy_get_window_size(procy_window_t *window, int *width, int *height);

void procy_get_glyph_size(procy_window_t *window, int *width, int *height);

void procy_set_clear_color(procy_color_t c);

void procy_set_window_title(procy_window_t *window, const char *title);

void procy_close_window(procy_window_t *window);

void procy_set_high_fps_mode(procy_window_t *window, bool high_fps);

void procy_set_scale(procy_window_t *window, float scale);

void procy_reset_scale(procy_window_t *window);

#endif
