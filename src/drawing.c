#include "drawing.h"

#include <limits.h>
#include <string.h>

#include "log.h"
#include "shader/sprite.h"
#include "window.h"

typedef procy_color_t color_t;
typedef procy_window_t window_t;
typedef procy_draw_op_text_t draw_op_text_t;
typedef procy_draw_op_rect_t draw_op_rect_t;
typedef procy_draw_op_sprite_t draw_op_sprite_t;
typedef procy_draw_op_line_t draw_op_line_t;

#define WHITE (procy_create_color(1.0F, 1.0F, 1.0F))
#define BLACK (procy_create_color(0.0F, 0.0F, 0.0F))

static void draw_string_chars(window_t *window, short x, short y, short z,
                              bool bold, color_t fg, color_t bg,
                              const char *contents) {
  int glyph_size;
  procy_get_glyph_size(window, &glyph_size, NULL);

  const size_t length = strnlen(contents, PROCY_MAX_DRAW_STRING_LENGTH);
  draw_op_text_t op;
  for (int i = 0; i < length; ++i) {
    op = procy_create_draw_op_char_colored(
        (short)((x + i * glyph_size) % SHRT_MAX), y, z, fg, bg, contents[i],
        bold);
    procy_append_draw_op_text(window, &op);
  }
}

procy_sprite_shader_program_t *procy_load_sprite_shader(window_t *window,
                                                        const char *path) {
  procy_sprite_shader_program_t *shader = procy_create_sprite_shader(path);

  if (shader == NULL) {
    log_error("Failed to load sprite shader with texture from \"%s\"", path);
    return NULL;
  }

  log_debug("Loaded sprite shader with texture from \"%s\"", path);
  procy_append_sprite_shader(window, shader);

  return shader;
}

procy_sprite_shader_program_t *procy_load_sprite_shader_mem(
    struct procy_window_t *window, unsigned char *buffer, size_t length) {
  procy_sprite_shader_program_t *shader =
      procy_create_sprite_shader_mem(buffer, length);

  if (shader == NULL) {
    log_error("Failed to load a sprite shader from an in-memory buffer");
    return NULL;
  }

  log_debug(
      "Loaded a sprite shader from an in-memory buffer %zu bytes in length",
      length);
  procy_append_sprite_shader(window, shader);

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

void procy_draw_string(window_t *window, short x, short y, short z,
                       color_t color, color_t background,
                       const char *contents) {
  draw_string_chars(window, x, y, z, false, color, background, contents);
}

void procy_draw_char(window_t *window, short x, short y, short z, color_t color,
                     color_t background, char c) {
  draw_op_text_t op =
      procy_create_draw_op_char_colored(x, y, z, color, background, c, false);
  procy_append_draw_op_text(window, &op);
}

void procy_draw_char_bold(window_t *window, short x, short y, short z,
                          color_t color, color_t background, char c) {
  draw_op_text_t op =
      procy_create_draw_op_char_colored(x, y, z, color, background, c, true);
  procy_append_draw_op_text(window, &op);
}

void procy_draw_string_bold(window_t *window, short x, short y, short z,
                            color_t color, color_t background,
                            const char *contents) {
  draw_string_chars(window, x, y, z, true, color, background, contents);
}

void procy_draw_rect(window_t *window, short x, short y, short z, short width,
                     short height, color_t color) {
  draw_op_rect_t op = procy_create_draw_op_rect(x, y, z, width, height, color);
  procy_append_draw_op_rect(window, &op);
}

void procy_draw_line(window_t *window, short x1, short y1, short x2, short y2,
                     short z, color_t color) {
  draw_op_line_t op = procy_create_draw_op_line(x1, y1, x2, y2, z, color);
  procy_append_draw_op_line(window, &op);
}

void procy_draw_sprite(window_t *window, short x, short y, short z,
                       procy_color_t color, procy_color_t background,
                       procy_sprite_t *sprite) {
  draw_op_sprite_t op =
      procy_create_draw_op_sprite(x, y, z, color, background, sprite);
  procy_append_draw_op_sprite(window, &op);
}

draw_op_text_t procy_create_draw_op_char(short x, short y, short z, char c,
                                         bool bold) {
  return procy_create_draw_op_char_colored(x, y, z, WHITE, BLACK, c, bold);
}

draw_op_text_t procy_create_draw_op_char_colored(short x, short y, short z,
                                                 color_t color,
                                                 color_t background, char c,
                                                 bool bold) {
  draw_op_text_t op = {color, background, x, y, z, c, bold};
  return op;
}

draw_op_rect_t procy_create_draw_op_rect(short x, short y, short z, short width,
                                         short height, color_t color) {
  draw_op_rect_t op = {color, x, y, z, width, height};
  return op;
}

draw_op_line_t procy_create_draw_op_line(short x1, short y1, short x2, short y2,
                                         short z, color_t color) {
  draw_op_line_t op = {color, x1, y1, x2, y2, z};
  return op;
}

draw_op_sprite_t procy_create_draw_op_sprite(short x, short y, short z,
                                             procy_color_t color,
                                             procy_color_t background,
                                             procy_sprite_t *sprite) {
  draw_op_sprite_t op = {color, background, x, y, z, sprite};
  return op;
}
