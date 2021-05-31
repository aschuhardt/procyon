#include "drawing.h"

#include <limits.h>
#include <string.h>

#include "log.h"
#include "shader/sprite.h"
#include "window.h"

typedef procy_draw_op_t draw_op_t;
typedef procy_color_t color_t;
typedef procy_window_t window_t;

#define WHITE (procy_create_color(1.0F, 1.0F, 1.0F))
#define BLACK (procy_create_color(0.0F, 0.0F, 0.0F))

#define PROCY_MAX_DRAW_STRING_LENGTH 256

static void draw_string_chars(window_t *window, short x, short y, bool bold,
                              color_t fg, color_t bg, const char *contents) {
  int glyph_size;
  procy_get_glyph_size(window, &glyph_size, NULL);

  const size_t length = strnlen(contents, PROCY_MAX_DRAW_STRING_LENGTH);
  draw_op_t draw_op;
  for (size_t i = 0; i < length; ++i) {
    draw_op = procy_create_draw_op_string_colored(x, y, glyph_size, fg, bg,
                                                  contents, i, bold);
    procy_append_draw_op(window, &draw_op);
  }
}

/* --------------------------- */
/* Public interface definition */
/* --------------------------- */

struct procy_sprite_shader_program_t *procy_load_sprite_shader(
    window_t *window, const char *path) {
  procy_sprite_shader_program_t **buffer = &window->shaders.sprite[0];
  for (int i = 0; i < MAX_SPRITE_SHADER_COUNT; ++i) {
    if (buffer[i] == 0) {
      procy_sprite_shader_program_t *shader = procy_create_sprite_shader(path);
      if (shader == NULL) {
        log_error("Failed to load a sprite shader \"%s\"", path);
        return NULL;
      }

      log_debug("Loaded sprite shader \"%s\" (index: %d)", path, i);
      buffer[i] = shader;

      return shader;
    }
  }

  // not enough room
  log_warn(
      "Attempted to load a new sprite shader \"%s\" but the maximum number of "
      "sprite shaders (%d) has been reached.",
      path, MAX_SPRITE_SHADER_COUNT);
  return NULL;
}

procy_sprite_t *procy_create_sprite(procy_sprite_shader_program_t *shader,
                                    short x, short y, short width,
                                    short height) {
  procy_sprite_t *sprite = malloc(sizeof(procy_sprite_t));
  if (sprite == NULL) {
    log_error("Failed to allocate memory for a sprite");
    return NULL;
  }

  sprite->shader = shader;
  sprite->x = x;
  sprite->y = y;
  sprite->width = width;
  sprite->height = height;
  return sprite;
}

void procy_destroy_sprite(procy_sprite_t *sprite) {
  if (sprite != NULL) {
    free(sprite);
  }
}

void procy_draw_string(window_t *window, short x, short y, color_t forecolor,
                       color_t backcolor, const char *contents) {
  draw_string_chars(window, x, y, false, forecolor, backcolor, contents);
}

void procy_draw_char(window_t *window, short x, short y, color_t forecolor,
                     color_t backcolor, char c) {
  draw_op_t op =
      procy_create_draw_op_char_colored(x, y, forecolor, backcolor, c, false);
  procy_append_draw_op(window, &op);
}

void procy_draw_char_bold(window_t *window, short x, short y, color_t forecolor,
                          color_t backcolor, char c) {
  draw_op_t op =
      procy_create_draw_op_char_colored(x, y, forecolor, backcolor, c, true);
  procy_append_draw_op(window, &op);
}

void procy_draw_string_bold(window_t *window, short x, short y,
                            color_t forecolor, color_t backcolor,
                            const char *contents) {
  draw_string_chars(window, x, y, true, forecolor, backcolor, contents);
}

void procy_draw_rect(window_t *window, short x, short y, short width,
                     short height, color_t color) {
  draw_op_t op = procy_create_draw_op_rect(x, y, width, height, color);
  procy_append_draw_op(window, &op);
}

void procy_draw_line(window_t *window, short x1, short y1, short x2, short y2,
                     color_t color) {
  draw_op_t op = procy_create_draw_op_line(x1, y1, x2, y2, color);
  procy_append_draw_op(window, &op);
}

void procy_draw_sprite(window_t *window, short x, short y,
                       procy_color_t forecolor, procy_color_t backcolor,
                       procy_sprite_t *sprite) {
  draw_op_t op =
      procy_create_draw_op_sprite(x, y, forecolor, backcolor, sprite);
  procy_append_draw_op(window, &op);
}

draw_op_t procy_create_draw_op_string(short x, short y, int size,
                                      const char *contents, size_t index,
                                      bool bold) {
  return procy_create_draw_op_char((short)((x + index * size) % SHRT_MAX), y,
                                   (char)contents[index], bold);
}

draw_op_t procy_create_draw_op_string_colored(short x, short y, int size,
                                              color_t forecolor,
                                              color_t backcolor,
                                              const char *contents,
                                              size_t index, bool bold) {
  return procy_create_draw_op_char_colored(
      (short)((x + index * size) % SHRT_MAX), y, forecolor, backcolor,
      (char)contents[index], bold);
}

draw_op_t procy_create_draw_op_char(short x, short y, char c, bool bold) {
  return procy_create_draw_op_char_colored(x, y, WHITE, BLACK, c, bold);
}

draw_op_t procy_create_draw_op_char_colored(short x, short y, color_t forecolor,
                                            color_t backcolor, char c,
                                            bool bold) {
  draw_op_t op = {forecolor, backcolor, DRAW_OP_TEXT, x, y};
  op.data.text.character = (unsigned char)c;
  op.data.text.bold = bold;
  return op;
}

procy_draw_op_t procy_create_draw_op_rect(short x, short y, short width,
                                          short height, color_t color) {
  draw_op_t op = {color, color, DRAW_OP_RECT, x, y};
  op.data.rect.width = width;
  op.data.rect.height = height;
  return op;
}

procy_draw_op_t procy_create_draw_op_line(short x1, short y1, short x2,
                                          short y2, color_t color) {
  draw_op_t op = {color, color, DRAW_OP_LINE, x1, y1};
  op.data.line.x2 = x2;
  op.data.line.y2 = y2;
  return op;
}

procy_draw_op_t procy_create_draw_op_sprite(short x, short y,
                                            procy_color_t forecolor,
                                            procy_color_t backcolor,
                                            procy_sprite_t *sprite) {
  draw_op_t op = {forecolor, backcolor, DRAW_OP_SPRITE, x, y};
  op.data.sprite.ptr = sprite;
  return op;
}
