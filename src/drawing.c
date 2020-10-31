#include "drawing.h"

#include <string.h>

#include "window.h"

typedef procy_draw_op_t draw_op_t;
typedef procy_color_t color_t;
typedef procy_window_t window_t;

#define WHITE (procy_create_color(1.0F, 1.0F, 1.0F))
#define BLACK (procy_create_color(0.0F, 0.0F, 0.0F))

#define PROCY_MAX_DRAW_STRING_LENGTH 256

static void draw_string_chars(window_t* window, int x, int y, bool bold,
                              bool vertical, color_t fg, color_t bg,
                              const char* contents) {
  int glyph_size;
  if (vertical) {
    procy_get_glyph_size(window, NULL, &glyph_size);
  } else {
    procy_get_glyph_size(window, &glyph_size, NULL);
  }

  size_t length = strnlen(contents, PROCY_MAX_DRAW_STRING_LENGTH);
  draw_op_t draw_op;
  for (int i = 0; i < length; ++i) {
    draw_op = procy_create_draw_op_string_colored(x, y, glyph_size, fg, bg,
                                                  contents, i, bold, vertical);
    procy_append_draw_op(window, &draw_op);
  }
}

/* --------------------------- */
/* Public interface definition */
/* --------------------------- */

void procy_draw_string(window_t* window, int x, int y, procy_color_t forecolor,
                       procy_color_t backcolor, const char* contents) {
  draw_string_chars(window, x, y, false, false, forecolor, backcolor, contents);
}

void procy_draw_string_bold(window_t* window, int x, int y,
                            procy_color_t forecolor, procy_color_t backcolor,
                            const char* contents) {
  draw_string_chars(window, x, y, true, false, forecolor, backcolor, contents);
}

void procy_draw_string_vertical(window_t* window, int x, int y,
                                procy_color_t forecolor,
                                procy_color_t backcolor, const char* contents) {
  draw_string_chars(window, x, y, false, true, forecolor, backcolor, contents);
}

void procy_draw_string_vertical_bold(window_t* window, int x, int y,
                                     procy_color_t forecolor,
                                     procy_color_t backcolor,
                                     const char* contents) {
  draw_string_chars(window, x, y, true, true, forecolor, backcolor, contents);
}

draw_op_t procy_create_draw_op_string(int x, int y, int size,
                                      const char* contents, size_t index,
                                      bool vertical, bool bold) {
  return vertical ? procy_create_draw_op_char(x, y + (int)index * size,
                                              (char)contents[index], bold)
                  : procy_create_draw_op_char(x + (int)index * size, y,
                                              (char)contents[index], bold);
}

draw_op_t procy_create_draw_op_string_colored(
    int x, int y, int size, color_t forecolor, color_t backcolor,
    const char* contents, size_t index, bool vertical, bool bold) {
  return vertical
             ? procy_create_draw_op_char_colored(x, y + (int)index * size,
                                                 forecolor, backcolor,
                                                 (char)contents[index], bold)

             : procy_create_draw_op_char_colored(x + (int)index * size, y,
                                                 forecolor, backcolor,
                                                 (char)contents[index], bold);
}

draw_op_t procy_create_draw_op_char(int x, int y, char c, bool bold) {
  return procy_create_draw_op_char_colored(x, y, WHITE, BLACK, c, bold);
}

draw_op_t procy_create_draw_op_char_colored(int x, int y, color_t forecolor,
                                            color_t backcolor, char c,
                                            bool bold) {
  draw_op_t op = {forecolor, backcolor, DRAW_OP_TEXT, x, y};
  op.data.text.character = (unsigned char)c;
  op.data.text.bold = bold;
  return op;
}

procy_draw_op_t procy_create_draw_op_rect(int x, int y, int width, int height,
                                          procy_color_t color) {
  draw_op_t op = {color, color, DRAW_OP_RECT, x, y};
  op.data.rect.width = width;
  op.data.rect.height = height;
  return op;
}

procy_draw_op_t procy_create_draw_op_line(int x1, int y1, int x2, int y2,
                                          procy_color_t color) {
  draw_op_t op = {color, color, DRAW_OP_LINE, x1, y1};
  op.data.line.x2 = x2;
  op.data.line.y2 = y2;
  return op;
}
