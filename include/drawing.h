#ifndef DRAWING_H
#define DRAWING_H

#define DRAW_OP_STRING_BUFFER_SIZE 64

typedef struct color_t {
  float r, g, b;
} color_t;

typedef enum draw_op_type_t { DRAW_OP_TEXT } draw_op_type_t;

typedef struct draw_op_t {
  color_t color;
  draw_op_type_t type;
  int x, y;
  union {
    struct {
      char contents[DRAW_OP_STRING_BUFFER_SIZE];
    } text;
  } data;
} draw_op_t;

draw_op_t create_draw_op_string(int x, int y, const char* contents);

draw_op_t create_draw_op_string_colored(int x, int y, color_t color,
                                        const char* contents);

#endif
