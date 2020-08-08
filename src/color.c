#include "color.h"

typedef procy_color_t color_t;

color_t procy_create_color(float r, float g, float b) {
  color_t c = {r, g, b};
  return c;
}
