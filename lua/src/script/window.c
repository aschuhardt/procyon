#include <log.h>
#include <lua.h>

#include "procyon.h"
#include "script.h"
#include "script/environment.h"

#define TBL_WINDOW "window"

#define FUNC_CLOSE "close"
#define FUNC_SIZE "size"
#define FUNC_GLYPH_SIZE "glyph_size"
#define FUNC_ON_DRAW "on_draw"
#define FUNC_ON_RESIZE "on_resize"
#define FUNC_ON_LOAD "on_load"
#define FUNC_ON_UNLOAD "on_unload"

static int close_window(lua_State* L) {
  if (!verify_arg_count(L, 0, __func__)) {
    return 0;
  }

  lua_getglobal(L, GLOBAL_WINDOW_PTR);
  procy_window_t* window = (procy_window_t*)lua_touserdata(L, -1);
  window->quitting = true;
  lua_pop(L, 1);
  return 0;
}

static int window_size(lua_State* L) {
  if (!verify_arg_count(L, 0, __func__)) {
    return 0;
  }

  lua_getglobal(L, GLOBAL_WINDOW_PTR);
  procy_window_t* window = (procy_window_t*)lua_touserdata(L, -1);
  lua_pushinteger(L, window->bounds.width);
  lua_pushinteger(L, window->bounds.height);
  return 2;
}

static int window_glyph_size(lua_State* L) {
  if (!verify_arg_count(L, 0, __func__)) {
    return 0;
  }

  lua_getglobal(L, GLOBAL_WINDOW_PTR);
  procy_window_t* window = (procy_window_t*)lua_touserdata(L, -1);
  lua_pushinteger(L, window->glyph.width);
  lua_pushinteger(L, window->glyph.height);
  return 2;
}

// called from the main window loop by way of script_env_t.on_draw
static void perform_draw(procy_state_t* const state) {
  lua_State* L = ((script_env_t*)state->data)->L;
  lua_getglobal(L, TBL_WINDOW);

  lua_getfield(L, -1, FUNC_ON_DRAW);
  if (lua_isfunction(L, -1)) {
    if (lua_pcall(L, 0, 0, 0) == LUA_ERRRUN) {
      log_error("Error calling %s.%s: %s", TBL_WINDOW, FUNC_ON_DRAW,
                lua_tostring(L, -1));
    }
  }

  lua_pop(L, 1);
}

static void handle_window_resized(procy_state_t* const state, int w, int h) {
  lua_State* L = ((script_env_t*)state->data)->L;
  lua_getglobal(L, TBL_WINDOW);

  lua_getfield(L, -1, FUNC_ON_RESIZE);
  if (lua_isfunction(L, -1)) {
    lua_pushinteger(L, w);
    lua_pushinteger(L, h);
    if (lua_pcall(L, 2, 0, 0) == LUA_ERRRUN) {
      log_error("Error calling %s.%s: %s", TBL_WINDOW, FUNC_ON_RESIZE,
                lua_tostring(L, -1));
    }
  }

  lua_pop(L, 1);
}

static void handle_window_loaded(procy_state_t* const state) {
  lua_State* L = ((script_env_t*)state->data)->L;
  lua_getglobal(L, TBL_WINDOW);

  lua_getfield(L, -1, FUNC_ON_LOAD);
  if (lua_isfunction(L, -1)) {
    if (lua_pcall(L, 0, 0, 0) == LUA_ERRRUN) {
      log_error("Error calling %s.%s: %s", TBL_WINDOW, FUNC_ON_LOAD,
                lua_tostring(L, -1));
    }
  }

  lua_pop(L, 1);
}

static void handle_window_unloaded(procy_state_t* const state) {
  lua_State* L = ((script_env_t*)state->data)->L;
  lua_getglobal(L, TBL_WINDOW);

  lua_getfield(L, -1, FUNC_ON_UNLOAD);
  if (lua_isfunction(L, -1)) {
    if (lua_pcall(L, 0, 0, 0) == LUA_ERRRUN) {
      log_error("Error calling %s.%s: %s", TBL_WINDOW, FUNC_ON_UNLOAD,
                lua_tostring(L, -1));
    }
  }

  lua_pop(L, 1);
}

void add_window(lua_State* L, script_env_t* env) {
  lua_newtable(L);

  env->state->on_draw = perform_draw;
  env->state->on_resize = handle_window_resized;
  env->state->on_load = handle_window_loaded;
  env->state->on_unload = handle_window_unloaded;

  env->state->on_draw = perform_draw;

  lua_pushcfunction(L, close_window);
  lua_setfield(L, -2, FUNC_CLOSE);

  lua_pushcfunction(L, window_size);
  lua_setfield(L, -2, FUNC_SIZE);

  lua_pushcfunction(L, window_glyph_size);
  lua_setfield(L, -2, FUNC_GLYPH_SIZE);

  lua_setglobal(L, TBL_WINDOW);
}
