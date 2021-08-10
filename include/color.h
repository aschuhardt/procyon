#ifndef COLOR_H
#define COLOR_H

#include <stdbool.h>

#define COLOR_TO_INT(c) (c.b | c.g << 8 | c.r << 16)

typedef struct procy_color_t {
  unsigned char r, g, b;
} procy_color_t;

procy_color_t procy_create_color(unsigned char r, unsigned char g,
                                 unsigned char b);

bool procy_colors_equal(procy_color_t *first, procy_color_t *second);

#endif
