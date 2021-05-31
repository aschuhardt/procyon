#include <lauxlib.h>
#include <limits.h>
#include <log.h>
#include <lua.h>
#include <math.h>

#include "procyon.h"
#include "script/environment.h"
#include "shader/sprite.h"

#define TBL_DRAWING "draw"
#define TBL_COLOR "color"
#define TBL_SPRITESHEET "spritesheet"
#define TBL_SPRITE_META "procyon_sprite_meta"

#define FUNC_DRAWSTRING "string"
#define FUNC_DRAWCHAR "char"
#define FUNC_DRAWRECT "rect"
#define FUNC_DRAWLINE "line"
#define FUNC_DRAWPOLY "poly"
#define FUNC_FROMRGB "from_rgb"
#define FUNC_LOADSPRITESHEET "load"
#define FUNC_CREATESPRITE "sprite"
#define FUNC_DRAWSPRITE "draw"
#define FIELD_SPRITESHEET_PTR "ptr"
#define FIELD_SPRITE_PTR "ptr"
#define FIELD_SPRITE_COLOR "color"
#define FIELD_SPRITE_BACKGROUND "background"

#define WHITE_RGB 1.0F, 1.0F, 1.0F
#define BLACK_RGB 0.0F, 0.0F, 0.0F
#define WHITE (procy_create_color(WHITE_RGB))
#define BLACK (procy_create_color(BLACK_RGB))

#ifndef M_PI
#define M_PI 3.14159265359
#endif

#ifndef M_PI_2
#define M_PI_2 1.57079632679
#endif

static int draw_string(lua_State *L) {
  lua_settop(L, 6);

  size_t length = 0;
  int x = lua_tointeger(L, 1);
  int y = lua_tointeger(L, 2);
  const char *contents = lua_tolstring(L, 3, &length);
  procy_color_t forecolor = luaL_opt(L, get_color, 5, WHITE);
  procy_color_t backcolor = luaL_opt(L, get_color, 6, BLACK);

  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);
  procy_window_t *window = (procy_window_t *)lua_touserdata(L, -1);

  int glyph_w = 0, glyph_h = 0;
  procy_get_glyph_size(window, &glyph_w, &glyph_h);

  bool bold = false;
  procy_draw_op_t op;
  for (size_t i = 0; i < length; i++) {
    // check for inline modifiers such as %b or %i
    if (contents[i] == '%' && i < length - 1) {
      // define the number of characters that will be skipped
      // note: this is used both for incrementing the character index (i +=
      // offset - 1), as well as for shifting text following the modifiers to
      // the by (offset * glyph width) in order to compensate for non-drawn
      // modifier characters
      int offset = 2;

      switch (contents[i + 1]) {
        case 'b':
          bold = !bold;
          break;
        case 'i': {
          procy_color_t tmp = forecolor;
          forecolor = backcolor;
          backcolor = tmp;
        } break;
        case '%':
          // escape the following '%' by skipping only the current one
          offset = 1;
          break;
        default:
          goto no_mod;
      }

      // offset the position in order to compensate for the
      // characters that are not being drawn
      x -= glyph_w * offset;

      // skip the modifier char which follows '%'
      i += offset - 1;
      continue;
    }

  no_mod:
    op = procy_create_draw_op_string_colored(x, y, glyph_w, forecolor,
                                             backcolor, contents, i, bold);
    procy_append_draw_op(window, &op);
  }

  return 0;
}

static int draw_char(lua_State *L) {
  lua_settop(L, 6);

  int x = lua_tointeger(L, 1);
  int y = lua_tointeger(L, 2);
  unsigned char value = lua_tointeger(L, 3) % UCHAR_MAX;
  procy_color_t forecolor = luaL_opt(L, get_color, 5, WHITE);
  procy_color_t backcolor = luaL_opt(L, get_color, 6, BLACK);

  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);
  procy_window_t *window = (procy_window_t *)lua_touserdata(L, -1);
  procy_draw_op_t op = procy_create_draw_op_char_colored(
      x, y, forecolor, backcolor, (char)value, false);
  procy_append_draw_op(window, &op);

  return 0;
}

static int draw_rect(lua_State *L) {
  lua_settop(L, 5);

  int x = lua_tointeger(L, 1);
  int y = lua_tointeger(L, 2);
  int w = lua_tointeger(L, 3);
  int h = lua_tointeger(L, 4);
  procy_color_t color = luaL_opt(L, get_color, 5, WHITE);

  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);
  procy_window_t *window = (procy_window_t *)lua_touserdata(L, -1);

  procy_draw_op_t op = procy_create_draw_op_rect(x, y, w, h, color);
  procy_append_draw_op(window, &op);

  return 0;
}

static int draw_line(lua_State *L) {
  lua_settop(L, 5);

  int x1 = lua_tointeger(L, 1);
  int y1 = lua_tointeger(L, 2);
  int x2 = lua_tointeger(L, 3);
  int y2 = lua_tointeger(L, 4);
  procy_color_t color = luaL_opt(L, get_color, 5, WHITE);

  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);
  procy_window_t *window = (procy_window_t *)lua_touserdata(L, -1);

  procy_draw_op_t op = procy_create_draw_op_line(x1, y1, x2, y2, color);
  procy_append_draw_op(window, &op);

  return 0;
}

static int draw_polygon(lua_State *L) {
  lua_settop(L, 5);

  int x = lua_tointeger(L, 1);
  int y = lua_tointeger(L, 2);
  float radius = lua_tonumber(L, 3);
  int n = lua_tointeger(L, 4);

  procy_color_t color = luaL_opt(L, get_color, 5, WHITE);

  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);
  procy_window_t *window = (procy_window_t *)lua_touserdata(L, -1);

  float interval = (2.0F * (float)M_PI) / (float)n;
  for (float theta = 0.0F; theta < 2.0F * M_PI; theta += interval) {
    float adjust = n % 2 == 0 ? 0 : (float)M_PI_2;
    float x1 = cosf(theta - adjust) * radius + (float)x;
    float y1 = sinf(theta - adjust) * radius + (float)y;
    float x2 = cosf(theta - adjust + interval) * radius + (float)x;
    float y2 = sinf(theta - adjust + interval) * radius + (float)y;

    procy_draw_op_t op =
        procy_create_draw_op_line((short)roundf(x1), (short)roundf(y1),
                                  (short)roundf(x2), (short)roundf(y2), color);
    procy_append_draw_op(window, &op);
  }

  return 0;
}

static int from_rgb(lua_State *L) {
  lua_settop(L, 3);

  float r = luaL_optnumber(L, 1, 0.0);
  float g = luaL_optnumber(L, 2, 0.0);
  float b = luaL_optnumber(L, 3, 0.0);

  push_color(L, r, g, b);

  return 1;
}

static int destroy_sprite(lua_State *L) {
  lua_getfield(L, 1, FIELD_SPRITE_PTR);
  if (lua_islightuserdata(L, -1)) {
    procy_sprite_t *sprite = lua_touserdata(L, -1);
    procy_destroy_sprite(sprite);
  }

  return 0;
}

static int draw_sprite(lua_State *L) {
  lua_settop(L, 3);

  short x = luaL_checkinteger(L, 2) % SHRT_MAX;
  short y = luaL_checkinteger(L, 3) % SHRT_MAX;

  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);
  procy_window_t *window = (procy_window_t *)lua_touserdata(L, -1);

  lua_getfield(L, 1, FIELD_SPRITE_PTR);
  procy_sprite_t *sprite = (procy_sprite_t *)lua_touserdata(L, -1);

  lua_getfield(L, 1, FIELD_SPRITE_COLOR);
  procy_color_t color = get_color(L, -1);

  lua_getfield(L, 1, FIELD_SPRITE_BACKGROUND);
  procy_color_t background = get_color(L, -1);

  procy_draw_sprite(window, x, y, color, background, sprite);

  return 0;
}

static void push_color_or_default(lua_State *L, int index, float r, float g,
                                  float b) {
  if (!lua_isnoneornil(L, index) && lua_istable(L, index)) {
    lua_pushvalue(L, index);
  } else {
    push_color(L, r, g, b);
  }
}

static int create_sprite(lua_State *L) {
  lua_settop(L, 7);

  lua_getfield(L, 1, FIELD_SPRITESHEET_PTR);
  procy_sprite_shader_program_t *shader =
      (procy_sprite_shader_program_t *)lua_touserdata(L, -1);

  short x = luaL_checkinteger(L, 2) % SHRT_MAX;
  short y = luaL_checkinteger(L, 3) % SHRT_MAX;
  short width = luaL_checkinteger(L, 4) % SHRT_MAX;
  short height = luaL_checkinteger(L, 5) % SHRT_MAX;

  procy_sprite_t *sprite = procy_create_sprite(shader, x, y, width, height);
  if (sprite == NULL) {
    LOG_SCRIPT_ERROR(L, "Failed to create sprite at (%s, %s)", x, y);
    return 0;
  }

  lua_newtable(L);

  lua_pushlightuserdata(L, sprite);
  lua_setfield(L, -2, FIELD_SPRITE_PTR);

  push_color_or_default(L, 6, WHITE_RGB);
  lua_setfield(L, -2, FIELD_SPRITE_COLOR);

  push_color_or_default(L, 7, BLACK_RGB);
  lua_setfield(L, -2, FIELD_SPRITE_BACKGROUND);

  lua_pushcfunction(L, draw_sprite);
  lua_setfield(L, -2, FUNC_DRAWSPRITE);

  // set metadata field so that sprites are cleaned up on gc
  luaL_setmetatable(L, TBL_SPRITE_META);

  return 1;
}

static int load_spritesheet(lua_State *L) {
  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);
  procy_window_t *window = (procy_window_t *)lua_touserdata(L, -1);

  procy_sprite_shader_program_t *shader =
      procy_load_sprite_shader(window, luaL_checkstring(L, 1));

  if (shader == NULL) {
    return 0;
  }

  lua_newtable(L);

  lua_pushlightuserdata(L, shader);
  lua_setfield(L, -2, FIELD_SPRITESHEET_PTR);

  lua_pushcfunction(L, create_sprite);
  lua_setfield(L, -2, FUNC_CREATESPRITE);

  return 1;
}

static void add_spritesheet(lua_State *L) {
  luaL_Reg methods[] = {{FUNC_LOADSPRITESHEET, load_spritesheet}, {NULL, NULL}};
  luaL_newlib(L, methods);
  lua_setglobal(L, TBL_SPRITESHEET);

  // register metatable for sprite garbage collection
  luaL_Reg metamethods[] = {{"__gc", destroy_sprite}, {NULL, NULL}};
  luaL_newlib(L, metamethods);
  lua_setfield(L, LUA_REGISTRYINDEX, TBL_SPRITE_META);
}

static void add_draw_ops(lua_State *L) {
  luaL_Reg methods[] = {
      {FUNC_DRAWSTRING, draw_string}, {FUNC_DRAWRECT, draw_rect},
      {FUNC_DRAWLINE, draw_line},     {FUNC_DRAWPOLY, draw_polygon},
      {FUNC_DRAWCHAR, draw_char},     {NULL, NULL}};
  luaL_newlib(L, methods);
  lua_setglobal(L, TBL_DRAWING);
}

static void add_color(lua_State *L) {
  luaL_Reg methods[] = {{FUNC_FROMRGB, from_rgb}, {NULL, NULL}};
  luaL_newlib(L, methods);
  lua_setglobal(L, TBL_COLOR);
}

void add_drawing(lua_State *L, script_env_t *env) {
  add_draw_ops(L);
  add_color(L);
  add_spritesheet(L);
}
