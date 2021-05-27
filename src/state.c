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

state_t *procy_create_state(void) {
  state_t *state = malloc(sizeof(state_t));
  state->data = NULL;
  state->on_load = NULL;
  state->on_unload = NULL;
  state->on_draw = NULL;
  state->on_resize = NULL;
  state->on_key_pressed = NULL;
  state->on_key_released = NULL;
  state->on_char_entered = NULL;
  return state;
}

void procy_destroy_state(state_t *state) {
  if (state != NULL) {
    free(state);
  }
}
