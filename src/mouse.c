#include "mouse.h"

#include <GLFW/glfw3.h>

procy_mouse_button_t procy_map_glfw_mouse_button(int button) {
  switch (button) {
    case GLFW_MOUSE_BUTTON_1:
      return MOUSE_BUTTON_LEFT;
    case GLFW_MOUSE_BUTTON_2:
      return MOUSE_BUTTON_RIGHT;
    case GLFW_MOUSE_BUTTON_3:
      return MOUSE_BUTTON_MIDDLE;
    case GLFW_MOUSE_BUTTON_4:
      return MOUSE_BUTTON_4;
    case GLFW_MOUSE_BUTTON_5:
      return MOUSE_BUTTON_5;
    case GLFW_MOUSE_BUTTON_6:
      return MOUSE_BUTTON_6;
    case GLFW_MOUSE_BUTTON_7:
      return MOUSE_BUTTON_7;
    case GLFW_MOUSE_BUTTON_8:
      return MOUSE_BUTTON_8;
    default:
      return MOUSE_BUTTON_0;
  }
}
