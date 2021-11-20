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

static int get_next_sprite_shader_index(procy_window_t *window) {
  procy_sprite_shader_program_t **buffer = &window->shaders.sprite[0];
  for (int i = 0; i < MAX_SPRITE_SHADER_COUNT; ++i) {
    if (buffer[i] == 0) {
      return i;
    }
  }

  // no room for more shaders
  return -1;
}

procy_sprite_shader_program_t *procy_load_sprite_shader(window_t *window,
                                                        const char *path) {
  // find the next index at which the new shader should be appended to the
  // window's sprite shader buffer
  int index = get_next_sprite_shader_index(window);

  // if adding a new shader would exceed the maximum allowed, the index will be
  // set to -1
  if (index < 0) {
    log_warn(
        "Attempted to load a new sprite shader \"%s\" but the maximum number "
        "of sprite shaders (%d) has been reached.",
        path, MAX_SPRITE_SHADER_COUNT);
    return NULL;
  }

  procy_sprite_shader_program_t *shader = procy_create_sprite_shader(path);

  if (shader != NULL) {
    window->shaders.sprite[index] = shader;
    log_debug("Loaded sprite shader \"%s\" (index: %d)", path, index);
  }

  return shader;
}

procy_sprite_shader_program_t *procy_load_sprite_shader_mem(
    struct procy_window_t *window, unsigned char *buffer, size_t length) {
  int index = get_next_sprite_shader_index(window);
  if (index < 0) {
    log_warn(
        "Attempted to load a new sprite shader from an in-memory buffer but "
        "the maximum nuber of sprite shaders (%d) has been reached",
        MAX_SPRITE_SHADER_COUNT);
    return NULL;
  }

  procy_sprite_shader_program_t *shader =
      procy_create_sprite_shader_mem(buffer, length);
  if (shader != NULL) {
    window->shaders.sprite[index] = shader;
    log_debug(
        "Loaded a sprite shader from an in-memory buffer %zu bytes in length "
        "(index: %d)",
        length, index);
  }

  return shader;
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

void procy_draw_string(window_t *window, short x, short y, color_t color,
                       color_t background, const char *contents) {
  draw_string_chars(window, x, y, false, color, background, contents);
}

void procy_draw_char(window_t *window, short x, short y, color_t color,
                     color_t background, char c) {
  draw_op_t op =
      procy_create_draw_op_char_colored(x, y, color, background, c, false);
  procy_append_draw_op(window, &op);
}

void procy_draw_char_bold(window_t *window, short x, short y, color_t color,
                          color_t background, char c) {
  draw_op_t op =
      procy_create_draw_op_char_colored(x, y, color, background, c, true);
  procy_append_draw_op(window, &op);
}

void procy_draw_string_bold(window_t *window, short x, short y, color_t color,
                            color_t background, const char *contents) {
  draw_string_chars(window, x, y, true, color, background, contents);
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

void procy_draw_sprite(window_t *window, short x, short y, procy_color_t color,
                       procy_color_t background, procy_sprite_t *sprite) {
  draw_op_t op = procy_create_draw_op_sprite(x, y, color, background, sprite);
  procy_append_draw_op(window, &op);
}

draw_op_t procy_create_draw_op_string(short x, short y, int size,
                                      const char *contents, size_t index,
                                      bool bold) {
  return procy_create_draw_op_char((short)((x + index * size) % SHRT_MAX), y,
                                   (char)contents[index], bold);
}

draw_op_t procy_create_draw_op_string_colored(short x, short y, int size,
                                              color_t color, color_t background,
                                              const char *contents,
                                              size_t index, bool bold) {
  return procy_create_draw_op_char_colored(
      (short)((x + index * size) % SHRT_MAX), y, color, background,
      (char)contents[index], bold);
}

draw_op_t procy_create_draw_op_char(short x, short y, char c, bool bold) {
  return procy_create_draw_op_char_colored(x, y, WHITE, BLACK, c, bold);
}

draw_op_t procy_create_draw_op_char_colored(short x, short y, color_t color,
                                            color_t background, char c,
                                            bool bold) {
  draw_op_t op = {color, DRAW_OP_TEXT, x, y};
  op.data.text.character = (unsigned char)c;
  op.data.text.bold = bold;
  op.data.text.background = background;
  return op;
}

procy_draw_op_t procy_create_draw_op_rect(short x, short y, short width,
                                          short height, color_t color) {
  draw_op_t op = {color, DRAW_OP_RECT, x, y};
  op.data.rect.width = width;
  op.data.rect.height = height;
  return op;
}

procy_draw_op_t procy_create_draw_op_line(short x1, short y1, short x2,
                                          short y2, color_t color) {
  draw_op_t op = {color, DRAW_OP_LINE, x1, y1};
  op.data.line.x2 = x2;
  op.data.line.y2 = y2;
  return op;
}

procy_draw_op_t procy_create_draw_op_sprite(short x, short y,
                                            procy_color_t color,
                                            procy_color_t background,
                                            procy_sprite_t *sprite) {
  draw_op_t op = {color, DRAW_OP_SPRITE, x, y};
  op.data.sprite.ptr = sprite;
  op.data.sprite.background = background;
  return op;
}
