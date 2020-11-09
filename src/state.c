#include "state.h"

#include <log.h>
#include <stdlib.h>

procy_state_t* procy_create_callback_state(
    procy_on_load_callback_t on_load, procy_on_unload_callback_t on_unload,
    procy_on_draw_callback_t on_draw, procy_on_resize_callback_t on_resize,
    procy_on_key_pressed_callback_t on_key_pressed,
    procy_on_key_released_callback_t on_key_released,
    procy_on_char_entered_callback_t on_char_entered) {
  procy_state_t* state = malloc(sizeof(procy_state_t));
  state->data = NULL;
  state->children = NULL;
  state->child_count = 0;
  state->on_load = on_load;
  state->on_unload = on_unload;
  state->on_draw = on_draw;
  state->on_resize = on_resize;
  state->on_key_pressed = on_key_pressed;
  state->on_key_released = on_key_released;
  state->on_char_entered = on_char_entered;
  return state;
}

void procy_append_child_state(procy_state_t* parent, procy_state_t* child) {
  parent->children =
      realloc(parent->children, sizeof(procy_state_t*) * ++parent->child_count);
  parent->children[parent->child_count - 1] = child;
}

void procy_destroy_callback_state(procy_state_t* state) {
  if (state != NULL) {
    free(state);
  }
}
