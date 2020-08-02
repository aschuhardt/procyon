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
#define FLD_COLOR_A "a"

#define FUNC_DRAWSTRING "string"
#define FUNC_FROMRGB "from_rgb"
#define FUNC_FROMRGBA "from_rgba"

static procy_color_t get_color(lua_State* L, int index) {
  if (!lua_istable(L, index)) {
    // if there's no table at `index`, return white
    return procy_create_color(255, 255, 255, 255);
  }

  unsigned char r, g, b, a;

  lua_getfield(L, index, FLD_COLOR_R);
  r = lua_isinteger(L, -1) ? (unsigned char)(lua_tointeger(L, -1) % 256) : 255;

  lua_getfield(L, index, FLD_COLOR_G);
  g = lua_isinteger(L, -1) ? (unsigned char)(lua_tointeger(L, -1) % 256) : 255;

  lua_getfield(L, index, FLD_COLOR_B);
  b = lua_isinteger(L, -1) ? (unsigned char)(lua_tointeger(L, -1) % 256) : 255;

  lua_getfield(L, index, FLD_COLOR_A);
  a = lua_isinteger(L, -1) ? (unsigned char)(lua_tointeger(L, -1) % 256) : 255;

  // pop color values + table
  lua_pop(L, 5);

  return procy_create_color(r, g, b, a);
}

static int draw_string(lua_State* L) {
  int x = lua_tointeger(L, 1);
  int y = lua_tointeger(L, 2);
  const char* contents = lua_tostring(L, 3);

  procy_color_t color = lua_gettop(L) == 4
                            ? get_color(L, 4)
                            : procy_create_color(255, 255, 255, 255);

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

static int get_normalized_color_value(lua_State* L, int index) {
  if (lua_isinteger(L, index)) {
    // assume that integers should be in the 0-255 range
    return lua_tointeger(L, index) % 256;
  } else if (lua_isnumber(L, index)) {
    float value = lua_tonumber(L, index);

    if (value >= 0.0F && value <= 1.0F) {
      // normalize values in the 0.0-1.0 range to 0-255
      return (int)truncf(value * 255.0F) % 256;
    }
  }

  // for invalid values, default to 255
  return 255;
}

static void push_rgba_table(lua_State* L, int r, int g, int b, int a) {
  // clear the stack
  lua_pop(L, lua_gettop(L));

  lua_newtable(L);

  lua_pushinteger(L, r);
  lua_pushinteger(L, g);
  lua_pushinteger(L, b);
  lua_pushinteger(L, a);

  lua_setfield(L, 1, FLD_COLOR_A);
  lua_setfield(L, 1, FLD_COLOR_B);
  lua_setfield(L, 1, FLD_COLOR_G);
  lua_setfield(L, 1, FLD_COLOR_R);
}

static int from_rgb(lua_State* L) {
  if (!verify_arg_count(L, 3, __func__)) {
    return 0;
  }

  int r = get_normalized_color_value(L, 1);
  int g = get_normalized_color_value(L, 2);
  int b = get_normalized_color_value(L, 3);

  push_rgba_table(L, r, g, b, 255);

  return 1;
}

static int from_rgba(lua_State* L) {
  if (!verify_arg_count(L, 4, __func__)) {
    return 0;
  }

  int r = get_normalized_color_value(L, 1);
  int g = get_normalized_color_value(L, 2);
  int b = get_normalized_color_value(L, 3);
  int a = get_normalized_color_value(L, 4);

  push_rgba_table(L, r, g, b, a);

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

  lua_pushcfunction(L, from_rgba);
  lua_setfield(L, -2, FUNC_FROMRGBA);

  lua_setglobal(L, TBL_COLOR);
}

void add_drawing(lua_State* L, script_env_t* env) {
  add_draw_ops_table(L);
  add_color_table(L);
}
