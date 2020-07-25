#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

#define CONFIG_SCRIPT_ENTRY_BUFFER_LENGTH 1024

typedef struct config_t {
  char script_entry[CONFIG_SCRIPT_ENTRY_BUFFER_LENGTH];
  int window_w, window_h;
  float glyph_scale;
} config_t;

bool parse_config_args(int argc, const char** argv, config_t* cfg);

#endif
