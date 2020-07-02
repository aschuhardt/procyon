#ifndef WINDOW_H
#define WINDOW_H

#include <stdbool.h>
#include <stdlib.h>

#define GLYPH_BUFFER_SIZE 2048

typedef struct glyph_t {
  int x, y;
  unsigned char data;
} glyph_t;

struct window_t;
struct config_t;

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

typedef struct window_t {
  glyph_t glyph_buffer[GLYPH_BUFFER_SIZE];
  tile_bounds_t tile_bounds;
  window_bounds_t bounds;
  draw_state_t state;
  size_t glyph_count;
  void* glfw_win;
  void* script_state;  // assigned upon script creation, for use during event
                       // callbacks (script_state_t)
  bool quitting;
} window_t;

window_t* create_window(struct config_t* cfg);

void destroy_window(window_t* window);

void begin_loop(window_t* window);

bool add_glyph_to_buffer(window_t* window, glyph_t op);

void set_state_dirty(window_t* w);

void set_state_wait(window_t* w);

#endif
