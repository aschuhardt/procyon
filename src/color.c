#include "color.h"

typedef procy_color_t color_t;

color_t procy_create_color(unsigned char r, unsigned char g, unsigned char b) {
  color_t c = {COLOR_TO_INT(r, g, b)};
  return c;
}

void procy_get_color_rgb(procy_color_t* color, unsigned char* r,
                         unsigned char* g, unsigned char* b) {
  *r = color->value >> 16 & 0xFF;
  *g = color->value >> 8 & 0xFF;
  *b = color->value & 0xFF;
}

bool procy_colors_equal(procy_color_t* first, procy_color_t* second) {
  return first->value == second->value;
}
