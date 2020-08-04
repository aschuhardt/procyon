#include <lua.h>

#include "script.h"
#include "script/environment.h"

void add_globals(lua_State* L, script_env_t* env) {
  lua_pushlightuserdata(L, env);
  lua_setglobal(L, GLOBAL_ENV_PTR);

  lua_pushlightuserdata(L, env->window);
  lua_setglobal(L, GLOBAL_WINDOW_PTR);
}
