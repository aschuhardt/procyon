#ifndef DRAWING_H
#define DRAWING_H

#include <stdbool.h>
#include <stddef.h>

#include "color.h"

struct procy_window_t;

typedef enum procy_draw_op_type_t {
  DRAW_OP_TEXT,
  DRAW_OP_RECT,
  DRAW_OP_LINE
} procy_draw_op_type_t;

typedef struct procy_draw_op_t {
  procy_color_t forecolor, backcolor;
  procy_draw_op_type_t type;
  short x, y;
  union {
    struct {
      unsigned char character;
      bool bold;
    } text;
    struct {
      short width, height;
    } rect;
    struct {
      short x2, y2;
    } line;
  } data;
} procy_draw_op_t;

void procy_draw_string(struct procy_window_t* window, short x, short y,
                       procy_color_t forecolor, procy_color_t backcolor,
                       const char* contents);

void procy_draw_string_bold(struct procy_window_t* window, short x, short y,
                            procy_color_t forecolor, procy_color_t backcolor,
                            const char* contents);

void procy_draw_rect(struct procy_window_t* window, short x, short y, short width,
                     short height, procy_color_t color);

void procy_draw_line(struct procy_window_t* window, short x1, short y1, short x2,
                     short y2, procy_color_t color);
/*
 * Returns a new text drawing operation for the character at the provided
 * index in a string whose first character's position is the given pixel
 * position
 */
procy_draw_op_t procy_create_draw_op_string(short x, short y, int size,
                                            const char* contents, size_t index,
                                            bool bold);
/*
 * Returns a new text drawing operation bearing the provided foreground color,
 * at the provided pixel coordinates
 */
procy_draw_op_t procy_create_draw_op_string_colored(
    short x, short y, int size, procy_color_t forecolor, procy_color_t backcolor,
    const char* contents, size_t index, bool bold);

procy_draw_op_t procy_create_draw_op_char(short x, short y, char c, bool bold);

procy_draw_op_t procy_create_draw_op_char_colored(short x, short y,
                                                  procy_color_t forecolor,
                                                  procy_color_t backcolor,
                                                  char c, bool bold);

procy_draw_op_t procy_create_draw_op_rect(short x, short y, short width, short height,
                                          procy_color_t color);

procy_draw_op_t procy_create_draw_op_line(short x1, short y1, short x2, short y2,
                                          procy_color_t color);

#endif
