#ifndef SCRIPT_H
#define SCRIPT_H

#include <stdbool.h>

struct lua_State;
struct procy_window_t;
struct script_env_t;
struct procy_state_t;

typedef struct script_env_t {
  struct lua_State* L;
  struct procy_window_t* window;
  struct procy_state_t* state;
  bool reload;  // when true, restart the in main after main loop ends
} script_env_t;

script_env_t* create_script_env(struct procy_window_t* window,
                                struct procy_state_t* state);

void destroy_script_env(script_env_t* env);

bool load_scripts(script_env_t* env, char* path);

#endif
