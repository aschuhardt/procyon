#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

#define CONFIG_SCRIPT_ENTRY_BUFFER_LENGTH 1024

typedef enum script_mode_t {
  SCRIPT_MODE_RAW,
  SCRIPT_MODE_PACKAGED
} script_mode_t;

typedef struct config_t {
  script_mode_t script_mode;
  char script_entry[CONFIG_SCRIPT_ENTRY_BUFFER_LENGTH];
  unsigned short window_w, window_h, tile_w, tile_h;
} config_t;

bool parse_args(int argc, const char** argv, config_t* cfg);

#endif
