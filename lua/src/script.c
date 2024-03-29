#include "script.h"

#include <lauxlib.h>
#include <libgen.h>
#include <log.h>
#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <stdbool.h>
#include <string.h>

#include "config.h"
#include "procyon.h"
#include "script/environment.h"

#ifdef _MSC_VER
#ifndef __INTEL_COMPILER
#define PATH_MAX 260
#endif
#endif

#define TBL_LIBRARY "pr"

static const char *get_lua_alloc_type_name(size_t t) {
  switch (t) {
    case LUA_TSTRING:
      return "String";
    case LUA_TTABLE:
      return "Table";
    case LUA_TFUNCTION:
      return "Function";
    case LUA_TUSERDATA:
      return "UserData";
    case LUA_TTHREAD:
      return "Thread";
    default:
      return "Other";
  }
}

static void add_package_path_from_entry(lua_State *L, char *path) {
  // get the "package" table
  lua_getglobal(L, "package");

  char *path_copy = strdup(path);
  const char *root = dirname(path_copy);

  // find and push the absolute directory of the provided file
  lua_pushfstring(L, "%s/?.lua;%s/?/init.lua;", root, root);

  free(path_copy);

  // push the value of the "path" record
  lua_getfield(L, -2, "path");

  // concatenate the directory and the existing path value, then assign
  // those to package.path
  lua_concat(L, 2);
  log_debug("Module package path: %s", lua_tostring(L, -1));
  lua_setfield(L, -2, "path");

  // pop the "package" table off the stack to leave it clean
  lua_pop(L, 1);
}

script_env_t *create_script_env(procy_window_t *window, procy_state_t *state) {
  script_env_t *env = malloc(sizeof(script_env_t));
  env->L = luaL_newstate();
  env->window = window;
  env->state = state;
  env->reload = false;
  return env;
}

void destroy_script_env(script_env_t *env) {
  if (env != NULL && env->L != NULL) {
    lua_close((lua_State *)env->L);
  }

  free(env);
}

bool load_scripts(script_env_t *env, char *path) {
  lua_State *L = (lua_State *)env->L;
  luaL_openlibs(L);

  luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON);
  add_package_path_from_entry(L, path);

  lua_pushlightuserdata(L, env);
  lua_setfield(L, LUA_REGISTRYINDEX, GLOBAL_ENV_PTR);

  lua_pushlightuserdata(L, env->window);
  lua_setfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);

  // ensure that the lib table will be at index 1
  // lua_pop(L, lua_gettop(L));

  lua_newtable(L);

  add_utilities(L);
  add_input(L, env);
  add_window(L, env);
  add_drawing(L, env);
  add_plane(L);
  add_noise(L);

  lua_setglobal(L, TBL_LIBRARY);

  if (luaL_dofile(L, path)) {
    log_error("Error loading file %s: %s", path, lua_tostring(L, -1));
    return false;
  }

  // script is loaded without errors
  return true;
}

void push_library_table(lua_State *L) { lua_getglobal(L, TBL_LIBRARY); }
