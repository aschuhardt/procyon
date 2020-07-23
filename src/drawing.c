#include "drawing.h"

#include <string.h>

const color_t WHITE = {1.0F, 1.0F, 1.0F};

draw_op_t create_draw_op_string(int x, int y, const char* contents) {
  draw_op_t op = {WHITE, DRAW_OP_TEXT, x, y};
  size_t len = strnlen(contents, sizeof(op.data.text.contents));
  memcpy(&op.data.text.contents[0], contents, len);
  op.data.text.contents[len] = '\0';
  return op;
}

draw_op_t create_draw_op_string_colored(int x, int y, color_t color,
                                        const char* contents) {
  draw_op_t op = create_draw_op_string(x, y, contents);
  op.color = color;
  return op;
}
