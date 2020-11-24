#ifndef COLOR_H
#define COLOR_H

#include <stdbool.h>

typedef struct procy_color_t {
  float r, g, b;
} procy_color_t;

procy_color_t procy_create_color(float r, float g, float b);

bool procy_colors_equal(procy_color_t* first, procy_color_t* second);

#endif
