#include "color.h"

#include <math.h>

typedef procy_color_t color_t;

#define FLOATS_EQUAL(a, b) (fabsf((a) - (b)) <= 0.0000001F)

/* --------------------------- */
/* Public interface definition */
/* --------------------------- */

color_t procy_create_color(float r, float g, float b) {
  color_t c = {r, g, b};
  return c;
}

bool procy_colors_equal(procy_color_t* first, procy_color_t* second) {
    return FLOATS_EQUAL(first->r, second->r) && FLOATS_EQUAL(first->g, second->g) && FLOATS_EQUAL(first->b, second->b);
}
