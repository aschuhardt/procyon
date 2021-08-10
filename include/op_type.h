#ifndef OP_TYPE_H
#define OP_TYPE_H

typedef enum procy_draw_op_type_t {
  DRAW_OP_TEXT = 1,
  DRAW_OP_RECT = 1 << 1,
  DRAW_OP_LINE = 1 << 2,
  DRAW_OP_SPRITE = 1 << 3
} procy_draw_op_type_t;

#endif
