#include <log.h>
#include <lua.h>
#include <math.h>
#include <string.h>

#include "procyon.h"
#include "script/environment.h"

#define TBL_DRAWING "draw"
#define TBL_COLOR "color"

#define FLD_COLOR_R "r"
#define FLD_COLOR_G "g"
#define FLD_COLOR_B "b"

#define FUNC_DRAWSTRING "string"
#define FUNC_FROMRGB "from_rgb"

#define WHITE procy_create_color(1.0F, 1.0F, 1.0F)

static procy_color_t get_color(lua_State* L, int index) {
  if (!lua_istable(L, index)) {
    // if there's no table at `index`, return white
    return WHITE;
  }

  float r, g, b, a;

  lua_getfield(L, index, FLD_COLOR_R);
  r = lua_tonumber(L, -1);

  lua_getfield(L, index, FLD_COLOR_G);
  g = lua_tonumber(L, -1);

  lua_getfield(L, index, FLD_COLOR_B);
  b = lua_tonumber(L, -1);

  // pop color values + table
  lua_pop(L, 5);

  return procy_create_color(r, g, b);
}

static int draw_string(lua_State* L) {
  int x = lua_tointeger(L, 1);
  int y = lua_tointeger(L, 2);
  const char* contents = lua_tostring(L, 3);

  procy_color_t color = lua_gettop(L) == 4 ? get_color(L, 4) : WHITE;

  lua_pop(L, lua_gettop(L));

  lua_getglobal(L, GLOBAL_WINDOW_PTR);
  procy_window_t* window = (procy_window_t*)lua_touserdata(L, -1);

  size_t length = strlen(contents);
  procy_draw_op_t op;
  for (size_t i = 0; i < length; i++) {
    op = procy_create_draw_op_string_colored(x, y, window->glyph.width, color,
                                             contents, i);
    procy_append_draw_op(window, &op);
  }

  return 0;
}

static void push_rgba_table(lua_State* L, float r, float g, float b) {
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

static int from_rgb(lua_State* L) {
  if (!verify_arg_count(L, 3, __func__)) {
    return 0;
  }

  float r = lua_tonumber(L, 1);
  float g = lua_tonumber(L, 2);
  float b = lua_tonumber(L, 3);

  push_rgba_table(L, r, g, b);

  return 1;
}

static void add_draw_ops_table(lua_State* L) {
  lua_newtable(L);

  lua_pushcfunction(L, draw_string);
  lua_setfield(L, -2, FUNC_DRAWSTRING);

  lua_setglobal(L, TBL_DRAWING);
}

static void add_color_table(lua_State* L) {
  lua_newtable(L);

  lua_pushcfunction(L, from_rgb);
  lua_setfield(L, -2, FUNC_FROMRGB);

  lua_setglobal(L, TBL_COLOR);
}

void add_drawing(lua_State* L, script_env_t* env) {
  add_draw_ops_table(L);
  add_color_table(L);
}
