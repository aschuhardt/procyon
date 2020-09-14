#include <log.h>
#include <lua.h>
#include <math.h>
#include <string.h>

#include "procyon.h"
#include "script/environment.h"

#define TBL_DRAWING "draw"
#define TBL_COLOR "color"

#define FUNC_DRAWSTRING "string"
#define FUNC_DRAWRECT "rect"
#define FUNC_DRAWLINE "line"
#define FUNC_DRAWPOLY "poly"
#define FUNC_FROMRGB "from_rgb"

#define WHITE (procy_create_color(1.0F, 1.0F, 1.0F))
#define BLACK (procy_create_color(0.0F, 0.0F, 0.0F))

#define MAX_DRAW_STRING_LENGTH 1024

static int draw_string(lua_State* L) {
  int x = lua_tointeger(L, 1);
  int y = lua_tointeger(L, 2);
  const char* contents = lua_tostring(L, 3);
  procy_color_t forecolor = lua_gettop(L) >= 4 ? get_color(L, 4) : WHITE;
  procy_color_t backcolor = lua_gettop(L) >= 5 ? get_color(L, 5) : BLACK;

  bool vertical = lua_gettop(L) >= 6 ? lua_toboolean(L, 6) : false;

  lua_pop(L, lua_gettop(L));

  lua_getglobal(L, GLOBAL_WINDOW_PTR);
  procy_window_t* window = (procy_window_t*)lua_touserdata(L, -1);

  size_t length = strnlen(contents, MAX_DRAW_STRING_LENGTH);
  int size = vertical ? window->glyph.height : window->glyph.width;
  procy_draw_op_t op;
  for (size_t i = 0; i < length; i++) {
    op = procy_create_draw_op_string_colored(x, y, size, forecolor, backcolor,
                                             contents, i, vertical);
    procy_append_draw_op(window, &op);
  }

  return 0;
}

static int draw_rect(lua_State* L) {
  int x = lua_tointeger(L, 1);
  int y = lua_tointeger(L, 2);
  int w = lua_tointeger(L, 3);
  int h = lua_tointeger(L, 4);

  procy_color_t color = lua_gettop(L) >= 5 ? get_color(L, 5) : WHITE;

  lua_pop(L, lua_gettop(L));

  lua_getglobal(L, GLOBAL_WINDOW_PTR);
  procy_window_t* window = (procy_window_t*)lua_touserdata(L, -1);

  procy_draw_op_t op = procy_create_draw_op_rect(x, y, w, h, color);
  procy_append_draw_op(window, &op);

  return 0;
}

static int draw_line(lua_State* L) {
  int x1 = lua_tointeger(L, 1);
  int y1 = lua_tointeger(L, 2);
  int x2 = lua_tointeger(L, 3);
  int y2 = lua_tointeger(L, 4);

  procy_color_t color = lua_gettop(L) >= 5 ? get_color(L, 5) : WHITE;

  lua_pop(L, lua_gettop(L));

  lua_getglobal(L, GLOBAL_WINDOW_PTR);
  procy_window_t* window = (procy_window_t*)lua_touserdata(L, -1);

  procy_draw_op_t op = procy_create_draw_op_line(x1, y1, x2, y2, color);
  procy_append_draw_op(window, &op);

  return 0;
}

static int draw_polygon(lua_State* L) {
  int x = lua_tointeger(L, 1);
  int y = lua_tointeger(L, 2);
  float radius = lua_tonumber(L, 3);
  int n = lua_tointeger(L, 4);

  procy_color_t color = lua_gettop(L) >= 5 ? get_color(L, 5) : WHITE;

  lua_pop(L, lua_gettop(L));

  lua_getglobal(L, GLOBAL_WINDOW_PTR);
  procy_window_t* window = (procy_window_t*)lua_touserdata(L, -1);

  float interval = (2.0F * (float)M_PI) / (float)n;
  for (float theta = 0.0F; theta < 2.0F * M_PI; theta += interval) {
    float adjust = n % 2 == 0 ? 0 : (float)M_PI_2;
    float x1 = cosf(theta - adjust) * radius + (float)x;
    float y1 = sinf(theta - adjust) * radius + (float)y;
    float x2 = cosf(theta - adjust + interval) * radius + (float)x;
    float y2 = sinf(theta - adjust + interval) * radius + (float)y;

    procy_draw_op_t op =
        procy_create_draw_op_line((int)roundf(x1), (int)roundf(y1),
                                  (int)roundf(x2), (int)roundf(y2), color);
    procy_append_draw_op(window, &op);
  }

  return 0;
}

static int from_rgb(lua_State* L) {
  if (!verify_arg_count(L, 3, __func__)) {
    return 0;
  }

  float r = lua_tonumber(L, 1);
  float g = lua_tonumber(L, 2);
  float b = lua_tonumber(L, 3);

  push_rgb_table(L, r, g, b);

  return 1;
}

static void add_draw_ops_table(lua_State* L) {
  lua_newtable(L);

  lua_pushcfunction(L, draw_string);
  lua_setfield(L, -2, FUNC_DRAWSTRING);

  lua_pushcfunction(L, draw_rect);
  lua_setfield(L, -2, FUNC_DRAWRECT);

  lua_pushcfunction(L, draw_line);
  lua_setfield(L, -2, FUNC_DRAWLINE);

  lua_pushcfunction(L, draw_polygon);
  lua_setfield(L, -2, FUNC_DRAWPOLY);

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
