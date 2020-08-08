#ifndef DRAWING_H
#define DRAWING_H

#include <stddef.h>

#include "color.h"

#define PROCY_DRAW_OP_STRING_BUFFER_SIZE 64

typedef enum procy_draw_op_type_t { DRAW_OP_TEXT } procy_draw_op_type_t;

typedef struct procy_draw_op_t {
  procy_color_t forecolor, backcolor;
  procy_draw_op_type_t type;
  int x, y;
  union {
    struct {
      float character;
    } text;
  } data;
} procy_draw_op_t;

/*
 * Returns a new text drawing operation for the character at the provided index
 * in a string whose first character's position is the given pixel position
 */
procy_draw_op_t procy_create_draw_op_string(int x, int y, int width,
                                            const char* contents, size_t index);
/*
 * Returns a new text drawing operation bearing the provided foreground color,
 * at the provided pixel coordinates
 */
procy_draw_op_t procy_create_draw_op_string_colored(int x, int y, int width,
                                                    procy_color_t forecolor,
                                                    procy_color_t backcolor,
                                                    const char* contents,
                                                    size_t index);

procy_draw_op_t procy_create_draw_op_char(int x, int y, char c);

procy_draw_op_t procy_create_draw_op_char_colored(int x, int y,
                                                  procy_color_t forecolor,
                                                  procy_color_t backcolor,
                                                  char c);

#endif
