#include "script/environment.h"

#include <log.h>
#include <lua.h>

#define FLD_COLOR_R "r"
#define FLD_COLOR_G "g"
#define FLD_COLOR_B "b"

procy_color_t get_color(lua_State *L, int index) {
  if (!lua_istable(L, index)) {
    // if there's no table at `index`, return white
    return procy_create_color(1.0F, 1.0F, 1.0F);
  }

  lua_getfield(L, index, FLD_COLOR_R);
  float r = lua_tonumber(L, -1);

  lua_getfield(L, index, FLD_COLOR_G);
  float g = lua_tonumber(L, -1);

  lua_getfield(L, index, FLD_COLOR_B);
  float b = lua_tonumber(L, -1);

  // pop color values
  lua_pop(L, 3);

  return procy_create_color(r, g, b);
}

void push_rgb_table(lua_State *L, float r, float g, float b) {
  // clear the stack
  lua_pop(L, lua_gettop(L));

  lua_newtable(L);

  lua_pushnumber(L, r);
  lua_setfield(L, -2, FLD_COLOR_R);

  lua_pushnumber(L, g);
  lua_setfield(L, -2, FLD_COLOR_G);

  lua_pushnumber(L, b);
  lua_setfield(L, -2, FLD_COLOR_B);
}

bool verify_arg_count(lua_State *L, int n, const char *name) {
  int arg_count = lua_gettop(L);
  if (arg_count < n) {
    lua_Debug debug;
    lua_getstack(L, 1, &debug);
    if (lua_getinfo(L, "Sl", &debug) != 0) {
      log_error(
          "%s:%d - invalid number of arguments passed to %s (expected %d, "
          "found %d)",
          debug.source, debug.currentline, name, n, arg_count);
    } else {
      log_error(
          "invalid number of arguments passed to %s (expected %d, found %d)",
          name, n, arg_count);
    }
    lua_error(L);
    return false;
  }
  return true;
}
