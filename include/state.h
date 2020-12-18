#ifndef STATE_H
#define STATE_H

#include <stdbool.h>
#include <stddef.h>

struct procy_state_t;
struct procy_key_info_t;

typedef void (*procy_on_load_callback_t)(struct procy_state_t *);
typedef void (*procy_on_unload_callback_t)(struct procy_state_t *);
typedef void (*procy_on_draw_callback_t)(struct procy_state_t *, double);
typedef void (*procy_on_resize_callback_t)(struct procy_state_t *, int, int);
typedef void (*procy_on_key_pressed_callback_t)(struct procy_state_t *,
                                                struct procy_key_info_t, bool,
                                                bool, bool);
typedef void (*procy_on_key_released_callback_t)(struct procy_state_t *,
                                                 struct procy_key_info_t, bool,
                                                 bool, bool);
typedef void (*procy_on_char_entered_callback_t)(struct procy_state_t *,
                                                 unsigned int);

typedef struct procy_state_t {
  void *data;
  struct procy_state_t *parent;
  struct procy_state_t **children;
  size_t child_count;
  procy_on_load_callback_t on_load;
  procy_on_unload_callback_t on_unload;
  procy_on_draw_callback_t on_draw;
  procy_on_resize_callback_t on_resize;
  procy_on_key_pressed_callback_t on_key_pressed;
  procy_on_key_released_callback_t on_key_released;
  procy_on_char_entered_callback_t on_char_entered;
} procy_state_t;

procy_state_t *procy_create_state();

procy_state_t *procy_create_callback_state(
    procy_on_load_callback_t on_load, procy_on_unload_callback_t on_unload,
    procy_on_draw_callback_t on_draw, procy_on_resize_callback_t on_resize,
    procy_on_key_pressed_callback_t on_key_pressed,
    procy_on_key_released_callback_t on_key_released,
    procy_on_char_entered_callback_t on_char_entered);

void procy_append_child_state(procy_state_t *parent, procy_state_t *child);

void procy_destroy_state(procy_state_t *state);

#endif
