#include <libgen.h>
#include <log.h>
#include <lua.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "script.h"
#include "script/environment.h"

#ifndef PATH_MAX
  #define PATH_MAX 1 << 16
#endif

#define FIELD_ROOT "ROOT"

void add_globals(lua_State *L, script_env_t *env, const char *path) {
  lua_pushlightuserdata(L, env);
  lua_setfield(L, LUA_REGISTRYINDEX, GLOBAL_ENV_PTR);

  lua_pushlightuserdata(L, env->window);
  lua_setfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);

  char abs_path[PATH_MAX] = {0};
  
#ifdef __MINGW32__
  _fullpath(path, &abs_path[0], sizeof(abs_path));
#else
  realpath(path, &abs_path[0]);
#endif

  if (strlen(&abs_path[0]) > 0) {
    const char *root = dirname(&abs_path[0]);
    lua_pushstring(L, root);
    lua_setglobal(L, FIELD_ROOT);
  } else {
    log_warn("Unable to determine the root script directory, so ROOT will be undefined");
  }
}
