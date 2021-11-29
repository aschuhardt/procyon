#ifndef COLOR_H
#define COLOR_H

#include <stdbool.h>

#define COLOR_TO_INT(r, g, b) ((b) | (g) << 8 | (r) << 16)

typedef struct procy_color_t {
  int value;
} procy_color_t;

void procy_get_color_rgb(procy_color_t* color, unsigned char* r,
                         unsigned char* g, unsigned char* b);

procy_color_t procy_create_color(unsigned char r, unsigned char g,
                                 unsigned char b);

bool procy_colors_equal(procy_color_t* first, procy_color_t* second);

#endif
