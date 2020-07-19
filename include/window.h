#ifndef WINDOW_H
#define WINDOW_H

#include <stdbool.h>
#include <stdlib.h>

struct window_t;
struct config_t;
struct draw_op_t;
struct GLFWwindow;
struct script_env_t;

typedef struct window_bounds_t {
  int width, height;
} window_bounds_t;

typedef struct glyph_bounds_t {
  int width, height;
} glyph_bounds_t;

typedef struct draw_op_buffer_t {
  struct draw_op_t* buffer;
  size_t length;
  size_t capacity;
} draw_op_buffer_t;

typedef struct window_t {
  draw_op_buffer_t draw_ops;
  window_bounds_t bounds;
  glyph_bounds_t glyph;
  unsigned int last_bound_texture;  // used to avoid re-binding textures
                                    // that are already bound
  float ortho[4][4];
  struct GLFWwindow* glfw_win;
  struct script_env_t* script_state;
  struct config_t* config;
  bool quitting;
} window_t;

window_t* create_window(struct config_t* cfg);

void destroy_window(window_t* window);

void begin_loop(window_t* window);

void append_string_draw_op(window_t* window, int x, int y,
                           const char* contents);

void set_window_bound_texture(window_t* w, unsigned int tex);

bool is_window_texture_bound(window_t* w, unsigned int tex);

#endif
