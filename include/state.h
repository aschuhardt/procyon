#ifndef STATE_H
#define STATE_H

#include <stdbool.h>

struct procy_state_t;
struct procy_key_info_t;
enum procy_mouse_mod_t;
enum procy_mouse_button_t;

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
typedef void (*procy_on_mouse_moved_callback_t)(struct procy_state_t *, double,
                                                double);
typedef void (*procy_on_mouse_pressed_callback_t)(struct procy_state_t *,
                                                  enum procy_mouse_button_t,
                                                  enum procy_mouse_mod_t);
typedef void (*procy_on_mouse_released_callback_t)(struct procy_state_t *,
                                                   enum procy_mouse_button_t,
                                                   enum procy_mouse_mod_t);

typedef struct procy_state_t {
  void *data;
  procy_on_load_callback_t on_load;
  procy_on_unload_callback_t on_unload;
  procy_on_draw_callback_t on_draw;
  procy_on_resize_callback_t on_resize;
  procy_on_key_pressed_callback_t on_key_pressed;
  procy_on_key_released_callback_t on_key_released;
  procy_on_char_entered_callback_t on_char_entered;
  procy_on_mouse_moved_callback_t on_mouse_moved;
  procy_on_mouse_pressed_callback_t on_mouse_pressed;
  procy_on_mouse_released_callback_t on_mouse_released;
} procy_state_t;

procy_state_t *procy_create_state(void);

procy_state_t *procy_create_callback_state(
    procy_on_load_callback_t on_load, procy_on_unload_callback_t on_unload,
    procy_on_draw_callback_t on_draw, procy_on_resize_callback_t on_resize,
    procy_on_key_pressed_callback_t on_key_pressed,
    procy_on_key_released_callback_t on_key_released,
    procy_on_char_entered_callback_t on_char_entered,
    procy_on_mouse_moved_callback_t on_mouse_moved,
    procy_on_mouse_pressed_callback_t on_mouse_pressed,
    procy_on_mouse_released_callback_t on_mouse_released);

void procy_destroy_state(procy_state_t *state);

#endif
