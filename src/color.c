#include "color.h"

typedef procy_color_t color_t;

/* --------------------------- */
/* Public interface definition */
/* --------------------------- */

color_t procy_create_color(unsigned char r, unsigned char g, unsigned char b) {
  color_t c = {r, g, b};
  return c;
}

bool procy_colors_equal(procy_color_t *first, procy_color_t *second) {
  return first->r == second->r && first->g == second->g &&
         first->b == second->b;
}
