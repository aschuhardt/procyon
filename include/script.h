#ifndef SCRIPT_H

#define SCRIPT_H

struct window_t;
enum script_mode_t;

typedef struct script_env_t {
  void* env;
  struct window_t* window;
} script_env_t;

typedef enum load_result_t { LOAD_SUCCESS, LOAD_FAILURE } load_result_t;

script_env_t* create_script_env(struct window_t* window);

void destroy_script_env(script_env_t* env);

load_result_t load_scripts(script_env_t* env, enum script_mode_t mode,
                           char* path);

#endif
