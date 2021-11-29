#ifndef MOUSE_H
#define MOUSE_H

typedef enum procy_mouse_button_t {
  MOUSE_BUTTON_0,
  MOUSE_BUTTON_1,
  MOUSE_BUTTON_2,
  MOUSE_BUTTON_3,
  MOUSE_BUTTON_4,
  MOUSE_BUTTON_5,
  MOUSE_BUTTON_6,
  MOUSE_BUTTON_7,
  MOUSE_BUTTON_8,
  MOUSE_BUTTON_LEFT = MOUSE_BUTTON_1,
  MOUSE_BUTTON_RIGHT = MOUSE_BUTTON_2,
  MOUSE_BUTTON_MIDDLE = MOUSE_BUTTON_3
} procy_mouse_button_t;

procy_mouse_button_t procy_map_glfw_mouse_button(int button);

#endif
