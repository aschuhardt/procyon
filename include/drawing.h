#ifndef DRAWING_H
#define DRAWING_H

#include <stddef.h>

#define PROCY_DRAW_OP_STRING_BUFFER_SIZE 64

typedef struct procy_color_t {
  unsigned int rgba;
} procy_color_t;

typedef enum procy_draw_op_type_t { DRAW_OP_TEXT } procy_draw_op_type_t;

typedef struct procy_draw_op_t {
  procy_color_t color;
  procy_draw_op_type_t type;
  int x, y;
  union {
    struct {
      unsigned char character;
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
                                                    procy_color_t color,
                                                    const char* contents,
                                                    size_t index);

procy_draw_op_t procy_create_draw_op_char(int x, int y, char c);

procy_draw_op_t procy_create_draw_op_char_colored(int x, int y,
                                                  procy_color_t color, char c);

/*
 * Encodes the four provided byte values (0 - 255) into a new procy_color_t
 * instance
 */
procy_color_t procy_create_color(unsigned char r, unsigned char g,
                                 unsigned char b, unsigned char a);

#endif
