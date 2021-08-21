#include <libgen.h>
#include <log.h>
#include <lua.h>
#include <stdlib.h>

#include "script.h"
#include "script/environment.h"

#define FLD_ROOT "ROOT"

void add_globals(lua_State *L, script_env_t *env, const char *path) {
  lua_pushlightuserdata(L, env);
  lua_setfield(L, LUA_REGISTRYINDEX, GLOBAL_ENV_PTR);

  lua_pushlightuserdata(L, env->window);
  lua_setfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);

  char *absolute = realpath(path, NULL);
  if (absolute == NULL) {
    log_error("Failed to obtain the absolute path of the script entry file");
    return;
  }

  const char *root = dirname(absolute);
  log_info("root: %s", root);

  lua_pushstring(L, root);
  lua_setglobal(L, FLD_ROOT);

  free(absolute);
}
