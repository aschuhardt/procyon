#ifndef DRAWING_H
#define DRAWING_H

#include <stdbool.h>
#include <stddef.h>

#include "color.h"

struct procy_window_t;

typedef struct procy_sprite_t {
  struct procy_sprite_shader_program_t *shader;
  int x, y, width, height;
} procy_sprite_t;

typedef struct procy_draw_op_text_t {
  procy_color_t color;
  procy_color_t background;
  int x, y, z;
  unsigned char character;
  bool bold;
} procy_draw_op_text_t;

typedef struct procy_draw_op_rect_t {
  procy_color_t color;
  int x, y, z;
  int width, height;
} procy_draw_op_rect_t;

typedef struct procy_draw_op_sprite_t {
  procy_color_t color;
  procy_color_t background;
  int x, y, z;
  procy_sprite_t *ptr;
} procy_draw_op_sprite_t;

typedef struct procy_draw_op_line_t {
  procy_color_t color;
  int x1, y1, x2, y2, z;
} procy_draw_op_line_t;

struct procy_sprite_shader_program_t *procy_load_sprite_shader(
    struct procy_window_t *window, const char *path);

struct procy_sprite_shader_program_t *procy_load_sprite_shader_mem(
    struct procy_window_t *window, unsigned char *buffer, size_t length);

procy_sprite_t *procy_create_sprite(
    struct procy_sprite_shader_program_t *shader, int x, int y, int width,
    int height);

void procy_destroy_sprite(procy_sprite_t *sprite);

void procy_draw_string(struct procy_window_t *window, int x, int y, int z,
                       procy_color_t color, procy_color_t background,
                       const char *contents);

void procy_draw_char(struct procy_window_t *window, int x, int y, int z,
                     procy_color_t color, procy_color_t background, char c);

void procy_draw_char_bold(struct procy_window_t *window, int x, int y,
                          int z, procy_color_t color,
                          procy_color_t background, char c);

void procy_draw_string_bold(struct procy_window_t *window, int x, int y,
                            int z, procy_color_t color,
                            procy_color_t background, const char *contents);

void procy_draw_rect(struct procy_window_t *window, int x, int y, int z,
                     int width, int height, procy_color_t color);

void procy_draw_line(struct procy_window_t *window, int x1, int y1,
                     int x2, int y2, int z, procy_color_t color);

void procy_draw_sprite(struct procy_window_t *window, int x, int y, int z,
                       procy_color_t color, procy_color_t background,
                       procy_sprite_t *sprite);

procy_draw_op_text_t procy_create_draw_op_char(int x, int y, int z,
                                               char c, bool bold);

procy_draw_op_text_t procy_create_draw_op_char_colored(int x, int y,
                                                       int z,
                                                       procy_color_t color,
                                                       procy_color_t background,
                                                       char c, bool bold);

procy_draw_op_rect_t procy_create_draw_op_rect(int x, int y, int z,
                                               int width, int height,
                                               procy_color_t color);

procy_draw_op_line_t procy_create_draw_op_line(int x1, int y1, int x2,
                                               int y2, int z,
                                               procy_color_t color);

procy_draw_op_sprite_t procy_create_draw_op_sprite(int x, int y, int z,
                                                   procy_color_t color,
                                                   procy_color_t background,
                                                   procy_sprite_t *sprite);
#endif
