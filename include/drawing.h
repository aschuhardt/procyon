#ifndef DRAWING_H
#define DRAWING_H

#define DRAW_OP_STRING_BUFFER_SIZE 128

typedef enum draw_op_type_t { DRAW_OP_TEXT } draw_op_type_t;

typedef struct draw_op_t {
  int x, y;
  draw_op_type_t type;
  union {
    struct {
      char contents[DRAW_OP_STRING_BUFFER_SIZE];
    } text;
  } data;
} draw_op_t;

draw_op_t create_draw_op_string(int x, int y, const char* contents);

#endif
