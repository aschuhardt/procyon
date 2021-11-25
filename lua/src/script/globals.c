#include <libgen.h>
#include <log.h>
#include <lua.h>
#include <stdlib.h>

#include "script.h"
#include "script/environment.h"

#define FIELD_ROOT "ROOT"

void add_globals(lua_State *L, script_env_t *env, const char *path) {
  lua_pushlightuserdata(L, env);
  lua_setfield(L, LUA_REGISTRYINDEX, GLOBAL_ENV_PTR);

  lua_pushlightuserdata(L, env->window);
  lua_setfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);

#ifdef __MINGW32__
  char *absolute = _fullpath(path, NULL, PATH_MAX);
#else
  char *absolute = realpath(path, NULL);
#endif

  if (absolute == NULL) {
    log_error("Failed to obtain the absolute path of the script entry file");
    return;
  }

  const char *root = dirname(absolute);

  lua_pushstring(L, root);
  lua_setglobal(L, FIELD_ROOT);

  free(absolute);
}
