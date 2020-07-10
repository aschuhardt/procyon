#ifndef WINDOW_H
#define WINDOW_H

#include <stdbool.h>
#include <stdlib.h>

struct window_t;
struct config_t;
struct draw_op_t;

typedef enum draw_state_t {
  WINDOW_STATE_WAIT,
  WINDOW_STATE_DIRTY
} draw_state_t;

typedef struct tile_bounds_t {
  int width, height;
} tile_bounds_t;

typedef struct window_bounds_t {
  int width, height;
} window_bounds_t;

typedef struct draw_op_buffer_t {
  struct draw_op_t* buffer;
  size_t length;
  size_t capacity;
} draw_op_buffer_t;

typedef struct window_t {
  draw_op_buffer_t draw_ops;
  tile_bounds_t tile_bounds;
  window_bounds_t bounds;
  draw_state_t state;
  unsigned int last_bound_texture;  // used to avoid re-binding already textures
                                    // that are already bound
  float ortho[4][4];
  void* glfw_win;
  void* script_state;  // assigned upon script creation, for use during event
                       // callbacks (script_state_t)
  void* glyph_shader;
  bool quitting;
} window_t;

window_t* create_window(struct config_t* cfg);

void destroy_window(window_t* window);

void begin_loop(window_t* window);

void append_string_draw_op(window_t* window, int x, int y,
                           const char* contents);

void set_window_state_dirty(window_t* w);

void set_window_state_wait(window_t* w);

void set_window_bound_texture(window_t* w, unsigned int tex);

bool is_window_texture_bound(window_t* w, unsigned int tex);

#endif
