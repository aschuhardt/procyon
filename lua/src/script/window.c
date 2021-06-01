#include <lauxlib.h>
#include <log.h>
#include <lua.h>

#include "procyon.h"
#include "script.h"
#include "script/environment.h"

#define TBL_WINDOW "window"

#define FUNC_CLOSE "close"
#define FUNC_RELOAD "reload"
#define FUNC_SIZE "size"
#define FUNC_SET_TITLE "set_title"
#define FUNC_GLYPH_SIZE "glyph_size"
#define FUNC_ON_DRAW "on_draw"
#define FUNC_ON_RESIZE "on_resize"
#define FUNC_ON_LOAD "on_load"
#define FUNC_ON_UNLOAD "on_unload"
#define FUNC_SET_COLOR "set_color"
#define FUNC_SET_HIGH_FPS "set_high_fps"
#define FUNC_SET_SCALE "set_scale"
#define FUNC_GET_SCALE "get_scale"
#define FUNC_RESET_SCALE "reset_scale"

static int close_window(lua_State *L) {
  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);
  procy_window_t *window = (procy_window_t *)lua_touserdata(L, -1);

  procy_close_window(window);

  return 0;
}

static int window_size(lua_State *L) {
  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);
  procy_window_t *window = (procy_window_t *)lua_touserdata(L, -1);

  int width = 0, height = 0;
  procy_get_window_size(window, &width, &height);

  lua_pushinteger(L, width);
  lua_pushinteger(L, height);

  return 2;
}

static int set_window_high_fps(lua_State *L) {
  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);

  ((procy_window_t *)lua_touserdata(L, -1))->high_fps =
      luaL_opt(L, lua_toboolean, 1, false);

  return 0;
}

static int window_glyph_size(lua_State *L) {
  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);
  procy_window_t *window = (procy_window_t *)lua_touserdata(L, -1);

  int width = 0, height = 0;
  procy_get_glyph_size(window, &width, &height);

  lua_pushinteger(L, width);
  lua_pushinteger(L, height);

  return 2;
}

static int window_reload(lua_State *L) {
  // close the window and set the "reload" flag to true
  close_window(L);

  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_ENV_PTR);
  script_env_t *env = (script_env_t *)lua_touserdata(L, -1);

  env->reload = true;

  return 0;
}

static int set_window_title(lua_State *L) {
  lua_settop(L, 1);

  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);
  procy_window_t *window = (procy_window_t *)lua_touserdata(L, -1);

  const char *title = luaL_checkstring(L, 1);

  procy_set_window_title(window, title);

  return 0;
}

// called from the main window loop by way of script_env_t.on_draw
static void perform_draw(procy_state_t *const state, double seconds) {
  lua_State *L = ((script_env_t *)state->data)->L;

  lua_getglobal(L, TBL_WINDOW);
  if (lua_isnoneornil(L, -1)) {
    log_warn("on_draw was called before the window table was initialized");
    return;
  }

  lua_getfield(L, -1, FUNC_ON_DRAW);
  if (lua_isfunction(L, -1)) {
    lua_pushnumber(L, seconds);
    if (lua_pcall(L, 1, 0, 0) == LUA_ERRRUN) {
      LOG_SCRIPT_ERROR(L, "Error calling %s.%s: %s", TBL_WINDOW, FUNC_ON_DRAW,
                       lua_tostring(L, -1));
    }

    // check whether the window is in "high fps" mode and, if it is NOT, run the
    // Lua garbage collector
    lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);
    procy_window_t *window = (procy_window_t *)lua_touserdata(L, -1);

    if (!window->high_fps) {
      lua_gc(L, LUA_GCCOLLECT);
    }
  }

  lua_pop(L, lua_gettop(L));
}

static void handle_window_resized(procy_state_t *const state, int w, int h) {
  lua_State *L = ((script_env_t *)state->data)->L;

  lua_getglobal(L, TBL_WINDOW);
  lua_getfield(L, -1, FUNC_ON_RESIZE);
  if (lua_isfunction(L, -1)) {
    lua_pushinteger(L, w);
    lua_pushinteger(L, h);
    if (lua_pcall(L, 2, 0, 0) == LUA_ERRRUN) {
      LOG_SCRIPT_ERROR(L, "Error calling %s.%s: %s", TBL_WINDOW, FUNC_ON_RESIZE,
                       lua_tostring(L, -1));
    }

    lua_gc(L, LUA_GCCOLLECT);
  }

  lua_pop(L, lua_gettop(L));
}

static void handle_window_loaded(procy_state_t *const state) {
  lua_State *L = ((script_env_t *)state->data)->L;

  lua_getglobal(L, TBL_WINDOW);
  lua_getfield(L, -1, FUNC_ON_LOAD);
  if (lua_isfunction(L, -1)) {
    if (lua_pcall(L, 0, 0, 0) == LUA_ERRRUN) {
      LOG_SCRIPT_ERROR(L, "Error calling %s.%s: %s", TBL_WINDOW, FUNC_ON_LOAD,
                       lua_tostring(L, -1));
    }

    lua_gc(L, LUA_GCCOLLECT);
  }

  lua_pop(L, lua_gettop(L));
}

static void handle_window_unloaded(procy_state_t *const state) {
  lua_State *L = ((script_env_t *)state->data)->L;

  lua_getglobal(L, TBL_WINDOW);
  lua_getfield(L, -1, FUNC_ON_UNLOAD);
  if (lua_isfunction(L, -1)) {
    if (lua_pcall(L, 0, 0, 0) == LUA_ERRRUN) {
      LOG_SCRIPT_ERROR(L, "Error calling %s.%s: %s", TBL_WINDOW, FUNC_ON_UNLOAD,
                       lua_tostring(L, -1));
    }

    lua_gc(L, LUA_GCCOLLECT);
  }

  lua_pop(L, lua_gettop(L));
}

static int set_window_color(lua_State *L) {
  lua_settop(L, 1);

  procy_set_clear_color(get_color(L, 1));

  return 0;
}

static int get_window_scale(lua_State *L) {
  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);
  procy_window_t *window = (procy_window_t *)lua_touserdata(L, -1);

  lua_pushnumber(L, window->scale);

  return 1;
}

static int set_window_scale(lua_State *L) {
  lua_settop(L, 1);
  float scale = luaL_checknumber(L, 1);

  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);
  procy_window_t *window = (procy_window_t *)lua_touserdata(L, -1);

  procy_set_scale(window, scale);

  return 0;
}

static int reset_window_scale(lua_State *L) {
  lua_getfield(L, LUA_REGISTRYINDEX, GLOBAL_WINDOW_PTR);
  procy_window_t *window = (procy_window_t *)lua_touserdata(L, -1);

  procy_reset_scale(window);

  return 0;
}

void add_window(lua_State *L, script_env_t *env) {
  env->state->on_draw = perform_draw;
  env->state->on_resize = handle_window_resized;
  env->state->on_load = handle_window_loaded;
  env->state->on_unload = handle_window_unloaded;
  env->state->on_draw = perform_draw;

  luaL_Reg methods[] = {{FUNC_CLOSE, close_window},
                        {FUNC_SIZE, window_size},
                        {FUNC_GLYPH_SIZE, window_glyph_size},
                        {FUNC_RELOAD, window_reload},
                        {FUNC_SET_COLOR, set_window_color},
                        {FUNC_SET_HIGH_FPS, set_window_high_fps},
                        {FUNC_SET_SCALE, set_window_scale},
                        {FUNC_GET_SCALE, get_window_scale},
                        {FUNC_RESET_SCALE, reset_window_scale},
                        {FUNC_SET_TITLE, set_window_title},
                        {NULL, NULL}};
  luaL_newlib(L, methods);
  lua_setglobal(L, TBL_WINDOW);
}
