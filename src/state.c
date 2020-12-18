#include "state.h"

#include <stdlib.h>

typedef procy_state_t state_t;

state_t *procy_create_callback_state(
    procy_on_load_callback_t on_load, procy_on_unload_callback_t on_unload,
    procy_on_draw_callback_t on_draw, procy_on_resize_callback_t on_resize,
    procy_on_key_pressed_callback_t on_key_pressed,
    procy_on_key_released_callback_t on_key_released,
    procy_on_char_entered_callback_t on_char_entered) {
  state_t *state = procy_create_state();
  state->on_load = on_load;
  state->on_unload = on_unload;
  state->on_draw = on_draw;
  state->on_resize = on_resize;
  state->on_key_pressed = on_key_pressed;
  state->on_key_released = on_key_released;
  state->on_char_entered = on_char_entered;
  return state;
}

state_t *procy_create_state() {
  state_t *state = malloc(sizeof(state_t));
  state->data = NULL;
  state->parent = NULL;
  state->children = NULL;
  state->child_count = 0;
  return state;
}

void procy_append_child_state(state_t *parent, state_t *child) {
  parent->children =
      realloc(parent->children, sizeof(state_t *) * ++parent->child_count);
  parent->children[parent->child_count - 1] = child;
  child->parent = parent;
}

void procy_destroy_state(state_t *state) {
  if (state == NULL) {
    return;
  }

  if (state->children != NULL) {
    // recursively destroy children
    for (size_t i = 0; i < state->child_count; ++i) {
      procy_destroy_state(state->children[i]);
    }

    free(state->children);
  }

  free(state);
}
