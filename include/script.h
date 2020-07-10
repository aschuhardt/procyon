#ifndef SCRIPT_H

#define SCRIPT_H

#include <stdbool.h>

struct window_t;

typedef struct script_env_t {
  void* env;
  struct window_t* window;
} script_env_t;

script_env_t* create_script_env(struct window_t* window);

void destroy_script_env(script_env_t* env);

bool load_scripts(script_env_t* env, char* path);

#endif
