#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

typedef struct config_t {
  char* script_entry;
  int window_w, window_h;
} config_t;

bool parse_config_args(int argc, const char** argv, config_t* cfg);

void destroy_config(config_t* cfg);

#endif
