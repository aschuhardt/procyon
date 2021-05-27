#include "script/environment.h"

#include <log.h>
#include <lua.h>

#define FIELD_COLOR_R "r"
#define FIELD_COLOR_G "g"
#define FIELD_COLOR_B "b"

procy_color_t get_color(lua_State *L, int index) {
  if (!lua_istable(L, index)) {
    // if there's no table at `index`, return white
    return procy_create_color(1.0F, 1.0F, 1.0F);
  }

  lua_getfield(L, index, FIELD_COLOR_R);
  float r = lua_tonumber(L, -1);

  lua_getfield(L, index, FIELD_COLOR_G);
  float g = lua_tonumber(L, -1);

  lua_getfield(L, index, FIELD_COLOR_B);
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
  lua_setfield(L, -2, FIELD_COLOR_R);

  lua_pushnumber(L, g);
  lua_setfield(L, -2, FIELD_COLOR_G);

  lua_pushnumber(L, b);
  lua_setfield(L, -2, FIELD_COLOR_B);
}

bool verify_arg_count(lua_State *L, int n, const char *name) {
  int arg_count = lua_gettop(L);
  if (arg_count < n) {
    LOG_SCRIPT_ERROR(
        L, "Invalid number of arguments passed to %s (expected %d, found %d)",
        n, arg_count);
    return false;
  }

  return true;
}
