#include "script/environment.h"

#include <lauxlib.h>
#include <lua.h>
#include <math.h>

#include "log.h"

#define FIELD_COLOR_R "r"
#define FIELD_COLOR_G "g"
#define FIELD_COLOR_B "b"

procy_color_t get_color(lua_State *L, int index) {
  lua_getfield(L, index, FIELD_COLOR_R);
  double r = luaL_optnumber(L, -1, 0.0F);
  lua_pop(L, 1);

  lua_getfield(L, index, FIELD_COLOR_G);
  double g = luaL_optnumber(L, -1, 0.0F);
  lua_pop(L, 1);

  lua_getfield(L, index, FIELD_COLOR_B);
  double b = luaL_optnumber(L, -1, 0.0F);
  lua_pop(L, 1);

  return procy_create_color((unsigned char)floor(r * 255.0),
                            (unsigned char)floor(g * 255.0),
                            (unsigned char)floor(b * 255.0));
}

void push_color(lua_State *L, float r, float g, float b) {
  // clear the stack
  lua_newtable(L);

  lua_pushnumber(L, r);
  lua_setfield(L, -2, FIELD_COLOR_R);

  lua_pushnumber(L, g);
  lua_setfield(L, -2, FIELD_COLOR_G);

  lua_pushnumber(L, b);
  lua_setfield(L, -2, FIELD_COLOR_B);
}
