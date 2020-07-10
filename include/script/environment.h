/*
 * This file contains declarations of methods that will set up various aspects
 * of the scripting environment.  The only file that should include this header
 * is `src/script.c`.
 */

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <stdbool.h>

#define GLOBAL_ENV_PTR "ptr_env"
#define GLOBAL_WINDOW_PTR "ptr_window"

typedef struct lua_State lua_State;
typedef struct script_env_t script_env_t;

void add_globals(lua_State* L, script_env_t* env);
void add_input(lua_State* L, script_env_t* env);
void add_drawing(lua_State* L, script_env_t* env);
void add_window(lua_State* L, script_env_t* env);
void add_utilities(lua_State* L);

/*
 * Utility methods available to script-setup logic
 */

bool verify_arg_count(lua_State* L, int n, const char* name);

#endif
