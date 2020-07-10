#include "drawing.h"

#include <string.h>

draw_op_t create_draw_op_string(int x, int y, const char* contents) {
  draw_op_t op = {x, y, DRAW_OP_TEXT};
  memset(&op.data.text.contents[0], '\0', sizeof(op.data.text.contents));
  memcpy(&op.data.text.contents[0], contents,
         strnlen(contents, sizeof(op.data.text.contents) - 1));
  return op;
}
