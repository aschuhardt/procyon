#include "drawing.h"

#include <string.h>

#include "window.h"

typedef procy_draw_op_t draw_op_t;
typedef procy_color_t color_t;

#define WHITE (procy_create_color(1.0F, 1.0F, 1.0F))
#define BLACK (procy_create_color(0.0F, 0.0F, 0.0F))

/* --------------------------- */
/* Public interface definition */
/* --------------------------- */

draw_op_t procy_create_draw_op_string(int x, int y, int width,
                                      const char* contents, size_t index) {
  return procy_create_draw_op_char(x + (int)index * width, y,
                                   (char)contents[index]);
}

draw_op_t procy_create_draw_op_string_colored(int x, int y, int width,
                                              color_t forecolor,
                                              color_t backcolor,
                                              const char* contents,
                                              size_t index) {
  return procy_create_draw_op_char_colored(x + (int)index * width, y, forecolor,
                                           backcolor, (char)contents[index]);
}

draw_op_t procy_create_draw_op_char(int x, int y, char c) {
  return procy_create_draw_op_char_colored(x, y, WHITE, BLACK, c);
}

draw_op_t procy_create_draw_op_char_colored(int x, int y, color_t forecolor,
                                            color_t backcolor, char c) {
  draw_op_t op = {forecolor, backcolor, DRAW_OP_TEXT, x, y};
  op.data.text.character = (unsigned char)c;
  return op;
}

