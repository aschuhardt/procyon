#ifndef COLOR_H
#define COLOR_H

typedef struct procy_color_t {
  float r, g, b;
} procy_color_t;

procy_color_t procy_create_color(float r, float g, float b);

#endif
