#ifndef SCRIPT_H
#define SCRIPT_H

#include <stdbool.h>

struct lua_State;
struct window_t;
struct script_env_t;

typedef void (*script_draw_routine_t)(struct script_env_t*);
typedef void (*script_window_resized_routine_t)(struct script_env_t*, int, int);
typedef void (*script_load_routine_t)(struct script_env_t*);
typedef void (*script_unload_routine_t)(struct script_env_t*);

typedef struct script_env_t {
  struct lua_State* L;
  struct window_t* window;

  // some functionality needs to be triggered by the window itself, such as
  // certain events which the window captures or the start + end of the script's
  // lifetime
  script_draw_routine_t on_draw;
  script_window_resized_routine_t on_resized;
  script_load_routine_t on_load;
  script_unload_routine_t on_unload;
} script_env_t;

script_env_t* create_script_env(struct window_t* window);

void destroy_script_env(script_env_t* env);

bool load_scripts(script_env_t* env, char* path);

#endif
