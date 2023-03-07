#include "drawing.h"

#include <lauxlib.h>
#include <log.h>
#include <lua.h>
#include <math.h>

#include "procyon.h"
#include "script/environment.h"
#include "shader/sprite.h"

#define GLOBAL_LAYER "procyon_layer"

#define TBL_DRAWING "draw"
#define TBL_COLOR "color"
#define TBL_SPRITESHEET "spritesheet"
#define TBL_SPRITE_META "procyon_sprite_meta"
#define TBL_SPRITESHEET_META "procyon_spritesheet_meta"

#define FUNC_DRAWSTRING "string"
#define FUNC_DRAWCHAR "char"
#define FUNC_DRAWRECT "rect"
#define FUNC_DRAWLINE "line"
#define FUNC_DRAWPOLY "poly"
#define FUNC_FROMRGB "from_rgb"
#define FUNC_LOADSPRITESHEET "load"
#define FUNC_CREATESPRITE "sprite"
#define FUNC_DRAWSPRITE "draw"
#define FUNC_SETLAYER "set_layer"
#define FIELD_SPRITESHEET_PTR "ptr"
#define FIELD_SPRITESHEET_WIDTH "width"
#define FIELD_SPRITESHEET_HEIGHT "height"
#define FIELD_SPRITE_COLOR "color"
#define FIELD_SPRITE_BACKGROUND "background"
#define FIELD_SPRITE_WIDTH "width"
#define FIELD_SPRITE_HEIGHT "height"
#define FIELD_SPRITE_X "x"
#define FIELD_SPRITE_Y "y"
#define FIELD_SPRITE_DATA "_data"
#define FIELD_RAWDATA_LENGTH "length"
#define FIELD_RAWDATA_BUFFER "buffer"

#define WHITE_RGB_FLOAT 1.0F, 1.0F, 1.0F
#define BLACK_RGB_FLOAT 0.0F, 0.0F, 0.0F
#define WHITE_RGB 255, 255, 255
#define BLACK_RGB 0, 0, 0
#define WHITE (procy_create_color(WHITE_RGB))
#define BLACK (procy_create_color(BLACK_RGB))

#ifndef M_PI
#define M_PI 3.14159265359
#endif

#ifndef M_PI_2
#define M_PI_2 1.57079632679
#endif

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
  lua_newtable(L);

  lua_pushnumber(L, r);
  lua_setfield(L, -2, FIELD_COLOR_R);

  lua_pushnumber(L, g);
  lua_setfield(L, -2, FIELD_COLOR_G);

  lua_pushnumber(L, b);
  lua_setfield(L, -2, FIELD_COLOR_B);
}
static int draw_string(lua_State *L) {
  lua_settop(L, 5);

  size_t length = 0;
  int x = (int)(lua_tointeger(L, 1));
  int y = (int)(lua_tointeger(L, 2));
  const char *contents = lua_tolstring(L, 3, &length);
  procy_color_t forecolor = luaL_opt(L, get_color, 4, WHITE);
  procy_color_t backcolor = luaL_opt(L, get_color, 5, BLACK);

  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_LAYER);
  int z = (int)(lua_tointeger(L, -1));

  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);
  procy_window_t *window = (procy_window_t *)lua_touserdata(L, -1);

  int glyph_w = 0;
  int glyph_h = 0;
  procy_get_glyph_size(window, &glyph_w, &glyph_h);

  bool bold = false;
  procy_draw_op_text_t op;
  for (size_t i = 0; i < length; i++) {
    // check for inline modifiers such as %b or %i
    if (contents[i] == '%' && i < length - 1) {
      // define the number of characters that will be skipped
      // note: this is used both for incrementing the character index (i +=
      // offset - 1), as well as for shifting text following the modifiers to
      // thz, e by (offset * glyph width) in order to compensate for non-drawn
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
      x = (int)((x - (glyph_w * offset)));

      // skip the modifier char which follows '%'
      i += offset - 1;
      continue;
    }

  no_mod:
    op = procy_create_draw_op_char_colored((int)((x + i * glyph_w)), y, z,
                                           forecolor, backcolor, contents[i],
                                           bold);
    procy_append_draw_op_text(window, &op);
  }

  return 0;
}

static int set_layer(lua_State *L) {
  lua_settop(L, 1);

  int z = (int)luaL_optinteger(L, 1, 0);

  lua_pushinteger(L, z);
  lua_setfield(L, LUA_REGISTRYINDEX, GLOBAL_LAYER);

  return 0;
}

static int draw_char(lua_State *L) {
  lua_settop(L, 5);

  int x = (int)(lua_tointeger(L, 1));
  int y = (int)(lua_tointeger(L, 2));
  unsigned char value = lua_tointeger(L, 3) % UCHAR_MAX;
  procy_color_t forecolor = luaL_opt(L, get_color, 4, WHITE);
  procy_color_t backcolor = luaL_opt(L, get_color, 5, BLACK);

  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_LAYER);
  int z = (int)(lua_tointeger(L, -1));

  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);
  procy_window_t *window = (procy_window_t *)lua_touserdata(L, -1);
  procy_draw_op_text_t op = procy_create_draw_op_char_colored(
      x, y, z, forecolor, backcolor, (char)value, false);
  procy_append_draw_op_text(window, &op);

  return 0;
}

static int draw_rect(lua_State *L) {
  lua_settop(L, 5);

  int x = (int)(lua_tointeger(L, 1));
  int y = (int)(lua_tointeger(L, 2));
  int w = (int)(lua_tointeger(L, 3));
  int h = (int)(lua_tointeger(L, 4));
  procy_color_t color = luaL_opt(L, get_color, 5, WHITE);

  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_LAYER);
  int z = (int)(lua_tointeger(L, -1));

  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);
  procy_window_t *window = (procy_window_t *)lua_touserdata(L, -1);

  procy_draw_op_rect_t op = procy_create_draw_op_rect(x, y, z, w, h, color);
  procy_append_draw_op_rect(window, &op);

  return 0;
}

static int draw_line(lua_State *L) {
  lua_settop(L, 5);

  int x1 = (int)(lua_tointeger(L, 1));
  int y1 = (int)(lua_tointeger(L, 2));
  int x2 = (int)(lua_tointeger(L, 3));
  int y2 = (int)(lua_tointeger(L, 4));
  procy_color_t color = luaL_opt(L, get_color, 5, WHITE);

  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_LAYER);
  int z = (int)(lua_tointeger(L, -1));

  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);
  procy_window_t *window = (procy_window_t *)lua_touserdata(L, -1);

  procy_draw_op_line_t op = procy_create_draw_op_line(x1, y1, x2, y2, z, color);
  procy_append_draw_op_line(window, &op);

  return 0;
}

static int draw_polygon(lua_State *L) {
  lua_settop(L, 5);

  int x = (int)lua_tointeger(L, 1);
  int y = (int)lua_tointeger(L, 2);
  float radius = (float)lua_tonumber(L, 3);
  int n = (int)lua_tointeger(L, 4);

  procy_color_t color = luaL_opt(L, get_color, 5, WHITE);

  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_LAYER);
  int z = (int)(lua_tointeger(L, -1));

  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);
  procy_window_t *window = (procy_window_t *)lua_touserdata(L, -1);

  float interval = (2.0F * (float)M_PI) / (float)n;
  for (float theta = 0.0F; theta < 2.0F * M_PI; theta += interval) {
    float adjust = n % 2 == 0 ? 0 : (float)M_PI_2;
    float x1 = cosf(theta - adjust) * radius + (float)x;
    float y1 = sinf(theta - adjust) * radius + (float)y;
    float x2 = cosf(theta - adjust + interval) * radius + (float)x;
    float y2 = sinf(theta - adjust + interval) * radius + (float)y;

    procy_draw_op_line_t op = procy_create_draw_op_line(
        (int)((int)roundf(x1)), (int)((int)roundf(y1)), (int)((int)roundf(x2)),
        (int)((int)roundf(y2)), z, color);
    procy_append_draw_op_line(window, &op);
  }

  return 0;
}

static int from_rgb(lua_State *L) {
  lua_settop(L, 3);

  float r = (float)luaL_optnumber(L, 1, 0.0);
  float g = (float)luaL_optnumber(L, 2, 0.0);
  float b = (float)luaL_optnumber(L, 3, 0.0);

  push_color(L, r, g, b);

  return 1;
}

static int draw_sprite(lua_State *L) {
  lua_settop(L, 5);
  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);
  procy_window_t *window = (procy_window_t *)lua_touserdata(L, -1);

  int x = (int)(luaL_checkinteger(L, 2));
  int y = (int)(luaL_checkinteger(L, 3));
  procy_color_t forecolor = luaL_opt(L, get_color, 4, WHITE);
  procy_color_t backcolor = luaL_opt(L, get_color, 5, BLACK);

  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_LAYER);
  int z = (int)(lua_tointeger(L, -1));

  lua_getfield(L, 1, FIELD_SPRITE_DATA);
  procy_sprite_t *sprite = (procy_sprite_t *)lua_touserdata(L, -1);

  procy_draw_sprite(window, x, y, z, forecolor, backcolor, sprite);

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
  lua_settop(L, 5);

  lua_getfield(L, 1, FIELD_SPRITESHEET_PTR);
  procy_sprite_shader_program_t *shader =
      (procy_sprite_shader_program_t *)lua_touserdata(L, -1);

  int x = (int)(luaL_checkinteger(L, 2));
  int y = (int)(luaL_checkinteger(L, 3));
  int width = (int)(luaL_checkinteger(L, 4));
  int height = (int)(luaL_checkinteger(L, 5));

  if (x < 0 || y < 0 || width <= 0 || height <= 0 ||
      x + width > shader->texture_w || y + height > shader->texture_h) {
    LOG_SCRIPT_ERROR(L, "Invalid sprite bounds");
    return 0;
  }

  lua_newtable(L);

  lua_pushinteger(L, x);
  lua_setfield(L, -2, FIELD_SPRITE_X);

  lua_pushinteger(L, y);
  lua_setfield(L, -2, FIELD_SPRITE_Y);

  lua_pushinteger(L, width);
  lua_setfield(L, -2, FIELD_SPRITE_WIDTH);

  lua_pushinteger(L, height);
  lua_setfield(L, -2, FIELD_SPRITE_HEIGHT);

  procy_sprite_t *sprite =
      (procy_sprite_t *)lua_newuserdata(L, sizeof(procy_sprite_t));
  sprite->x = x;
  sprite->y = y;
  sprite->width = width;
  sprite->height = height;
  sprite->shader = shader;
  lua_setfield(L, -2, FIELD_SPRITE_DATA);

  luaL_setmetatable(L, TBL_SPRITE_META);

  return 1;
}

static void push_spritesheet_table(lua_State *L,
                                   procy_sprite_shader_program_t *shader) {
  lua_newtable(L);

  lua_pushlightuserdata(L, shader);
  lua_setfield(L, -2, FIELD_SPRITESHEET_PTR);

  lua_pushinteger(L, shader->texture_w);
  lua_setfield(L, -2, FIELD_SPRITESHEET_WIDTH);

  lua_pushinteger(L, shader->texture_h);
  lua_setfield(L, -2, FIELD_SPRITESHEET_HEIGHT);

  luaL_setmetatable(L, TBL_SPRITESHEET_META);
}

static int load_spritesheet_raw(lua_State *L) {
  // accepts a table
  // the table has a "buffer" field and a "length" field
  lua_getfield(L, 1, FIELD_RAWDATA_LENGTH);
  size_t length = luaL_checkinteger(L, -1);

  lua_getfield(L, 1, FIELD_RAWDATA_BUFFER);
  if (!lua_islightuserdata(L, -1)) {
    lua_pushnil(L);
    return 1;
  }

  unsigned char *buffer = lua_touserdata(L, -1);

  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);
  procy_window_t *window = (procy_window_t *)lua_touserdata(L, -1);

  procy_sprite_shader_program_t *shader =
      procy_load_sprite_shader_mem(window, buffer, length);

  if (shader == NULL) {
    return 0;
  }

  push_spritesheet_table(L, shader);

  return 1;
}

static int load_spritesheet(lua_State *L) {
  lua_settop(L, 1);

  if (lua_istable(L, 1)) {
    // load from a raw buffer object instead of from a file
    return load_spritesheet_raw(L);
  }

  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);
  procy_window_t *window = (procy_window_t *)lua_touserdata(L, -1);

  procy_sprite_shader_program_t *shader =
      procy_load_sprite_shader(window, luaL_checkstring(L, 1));

  if (shader == NULL) {
    return 0;
  }

  push_spritesheet_table(L, shader);

  return 1;
}

static void add_sprite(lua_State *L) {
  if (luaL_newmetatable(L, TBL_SPRITE_META)) {
    luaL_Reg index[] = {{FUNC_DRAWSPRITE, draw_sprite}, {NULL, NULL}};
    luaL_newlib(L, index);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
  }
}

static void add_spritesheet(lua_State *L) {
  luaL_Reg methods[] = {{FUNC_LOADSPRITESHEET, load_spritesheet}, {NULL, NULL}};
  luaL_newlib(L, methods);
  lua_setfield(L, 1, TBL_SPRITESHEET);

  if (luaL_newmetatable(L, TBL_SPRITESHEET_META)) {
    luaL_Reg index[] = {{FUNC_CREATESPRITE, create_sprite}, {NULL, NULL}};
    luaL_newlib(L, index);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
  }
}

static void add_draw_ops(lua_State *L) {
  luaL_Reg methods[] = {{FUNC_DRAWSTRING, draw_string},
                        {FUNC_DRAWRECT, draw_rect},
                        {FUNC_DRAWLINE, draw_line},
                        {FUNC_DRAWPOLY, draw_polygon},
                        {FUNC_DRAWCHAR, draw_char},
                        {FUNC_SETLAYER, set_layer},
                        {NULL, NULL}};
  luaL_newlib(L, methods);
  lua_setfield(L, 1, TBL_DRAWING);

  lua_pushinteger(L, 1);
  lua_setfield(L, LUA_REGISTRYINDEX, GLOBAL_LAYER);
}

static void add_color(lua_State *L) {
  luaL_Reg methods[] = {{FUNC_FROMRGB, from_rgb}, {NULL, NULL}};
  luaL_newlib(L, methods);
  lua_setfield(L, 1, TBL_COLOR);
}

void add_drawing(lua_State *L, script_env_t *env) {
  add_draw_ops(L);
  add_color(L);
  add_spritesheet(L);
  add_sprite(L);
}
