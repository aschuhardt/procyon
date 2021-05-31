#include <lua.h>

#include "script.h"
#include "script/environment.h"

void add_globals(lua_State *L, script_env_t *env) {
  lua_pushlightuserdata(L, env);
  lua_setfield(L, LUA_REGISTRYINDEX, GLOBAL_ENV_PTR);

  lua_pushlightuserdata(L, env->window);
  lua_setfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);
}
