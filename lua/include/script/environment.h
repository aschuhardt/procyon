/*
 * This file contains declarations of methods that will set up various aspects
 * of the scripting environment.  The only file that should include this header
 * is `src/script.c`.
 */

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "color.h"

#define GLOBAL_ENV_PTR "procyon_ptr_env"
#define GLOBAL_WINDOW_PTR "procyon_ptr_window"

typedef struct lua_State lua_State;
typedef struct script_env_t script_env_t;
typedef struct state_t state_t;

void add_input(lua_State *L, script_env_t *env);
void add_drawing(lua_State *L, script_env_t *env);
void add_window(lua_State *L, script_env_t *env);
void add_utilities(lua_State *L);
void add_noise(lua_State *L);
void add_plane(lua_State *L);

/*
 * Utility methods available to script-setup logic
 */

void push_color(lua_State *L, float r, float g, float b);

procy_color_t get_color(lua_State *L, int index);

#ifdef _WIN32
#define PROCY_SCRIPT_PATH_SEPARATOR '\\'
#else
#define PROCY_SCRIPT_PATH_SEPARATOR '/'
#endif

#define LOG_SCRIPT(L, fmt, type, ...)                                \
  {                                                                  \
    lua_Debug debug;                                                 \
    if (lua_getstack(L, 1, &debug) != 0 &&                           \
        lua_getinfo(L, "nSl", &debug) != 0) {                        \
      char *filename =                                               \
          strrchr(debug.short_src, PROCY_SCRIPT_PATH_SEPARATOR) + 1; \
      if (debug.name == NULL) {                                      \
        log_##type("[%s:%d] " fmt, filename, debug.currentline,      \
                   ##__VA_ARGS__);                                   \
      } else {                                                       \
        log_##type("[%s:%d (%s)] " fmt, filename, debug.currentline, \
                   debug.name, ##__VA_ARGS__);                       \
      }                                                              \
    } else {                                                         \
      log_##type(fmt, ##__VA_ARGS__);                                \
    }                                                                \
  }

#define LOG_SCRIPT_ERROR(L, fmt, ...) LOG_SCRIPT(L, fmt, error, ##__VA_ARGS__)

#define LOG_SCRIPT_WARN(L, fmt, ...) LOG_SCRIPT(L, fmt, warn, ##__VA_ARGS__)

#define LOG_SCRIPT_DEBUG(L, fmt, ...) LOG_SCRIPT(L, fmt, debug, ##__VA_ARGS__)

#endif
