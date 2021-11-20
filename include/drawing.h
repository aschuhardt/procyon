#ifndef DRAWING_H
#define DRAWING_H

#include <stdbool.h>
#include <stddef.h>

#include "color.h"
#include "op_type.h"

struct procy_window_t;
struct procy_sprite_shader_program_t;

typedef struct procy_sprite_t {
  struct procy_sprite_shader_program_t *shader;
  short x, y, width, height;
} procy_sprite_t;

typedef struct procy_draw_op_t {
  procy_color_t color;
  procy_draw_op_type_t type;
  short x, y;
  union {
    struct {
      procy_color_t background;
      unsigned char character;
      bool bold;
    } text;
    struct {
      short width, height;
    } rect;
    struct {
      procy_color_t background;
      procy_sprite_t *ptr;
    } sprite;
    struct {
      short x2, y2;
    } line;
  } data;
} procy_draw_op_t;

struct procy_sprite_shader_program_t *procy_load_sprite_shader(
    struct procy_window_t *window, const char *path);

struct procy_sprite_shader_program_t *procy_load_sprite_shader_mem(
    struct procy_window_t *window, unsigned char *buffer, size_t length);

procy_sprite_t *procy_create_sprite(
    struct procy_sprite_shader_program_t *shader, short x, short y, short width,
    short height);

void procy_destroy_sprite(procy_sprite_t *sprite);

void procy_draw_string(struct procy_window_t *window, short x, short y,
                       procy_color_t color, procy_color_t background,
                       const char *contents);

void procy_draw_char(struct procy_window_t *window, short x, short y,
                     procy_color_t color, procy_color_t background, char c);

void procy_draw_char_bold(struct procy_window_t *window, short x, short y,
                          procy_color_t color, procy_color_t background,
                          char c);

void procy_draw_string_bold(struct procy_window_t *window, short x, short y,
                            procy_color_t color, procy_color_t background,
                            const char *contents);

void procy_draw_rect(struct procy_window_t *window, short x, short y,
                     short width, short height, procy_color_t color);

void procy_draw_line(struct procy_window_t *window, short x1, short y1,
                     short x2, short y2, procy_color_t color);

void procy_draw_sprite(struct procy_window_t *window, short x, short y,
                       procy_color_t color, procy_color_t background,
                       procy_sprite_t *sprite);

/*
 * Returns a new text drawing operation for the character at the provided
 * index in a string whose first character's position is the given pixel
 * position
 */
procy_draw_op_t procy_create_draw_op_string(short x, short y, int size,
                                            const char *contents, size_t index,
                                            bool bold);
/*
 * Returns a new text drawing operation bearing the provided foreground color,
 * at the provided pixel coordinates
 */
procy_draw_op_t procy_create_draw_op_string_colored(short x, short y, int size,
                                                    procy_color_t color,
                                                    procy_color_t background,
                                                    const char *contents,
                                                    size_t index, bool bold);

procy_draw_op_t procy_create_draw_op_char(short x, short y, char c, bool bold);

procy_draw_op_t procy_create_draw_op_char_colored(short x, short y,
                                                  procy_color_t color,
                                                  procy_color_t background,
                                                  char c, bool bold);

procy_draw_op_t procy_create_draw_op_rect(short x, short y, short width,
                                          short height, procy_color_t color);

procy_draw_op_t procy_create_draw_op_line(short x1, short y1, short x2,
                                          short y2, procy_color_t color);

procy_draw_op_t procy_create_draw_op_sprite(short x, short y,
                                            procy_color_t color,
                                            procy_color_t background,
                                            procy_sprite_t *sprite);
#endif
