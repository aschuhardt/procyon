#include "drawing.h"

#include <string.h>

#include "window.h"

typedef procy_draw_op_t draw_op_t;
typedef procy_color_t color_t;

#define WHITE (procy_create_color(255, 255, 255, 255))

/* --------------------------- */
/* Public interface definition */
/* --------------------------- */

draw_op_t procy_create_draw_op_string(int x, int y, int width,
                                      const char* contents, size_t index) {
  return procy_create_draw_op_char(x + (int)index * width, y,
                                   (char)contents[index]);
}

draw_op_t procy_create_draw_op_string_colored(int x, int y, int width,
                                              color_t color,
                                              const char* contents,
                                              size_t index) {
  return procy_create_draw_op_char_colored(x + (int)index * width, y, color,
                                           (char)contents[index]);
}

draw_op_t procy_create_draw_op_char(int x, int y, char c) {
  return procy_create_draw_op_char_colored(x, y, WHITE, c);
}

draw_op_t procy_create_draw_op_char_colored(int x, int y, color_t color,
                                            char c) {
  draw_op_t op = {WHITE, DRAW_OP_TEXT, x, y};
  op.data.text.character = (unsigned char)c;
  return op;
}

color_t procy_create_color(unsigned char r, unsigned char g, unsigned char b,
                           unsigned char a) {
  color_t color = {(unsigned int)r};
  color.rgba = (color.rgba << 8) | g;
  color.rgba = (color.rgba << 8) | b;
  color.rgba = (color.rgba << 8) | a;
  return color;
}

