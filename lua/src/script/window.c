#include <log.h>
#include <lua.h>

#include "procyon.h"
#include "script.h"
#include "script/environment.h"

#define TBL_WINDOW "window"

#define FUNC_CLOSE "close"
#define FUNC_RELOAD "reload"
#define FUNC_SIZE "size"
#define FUNC_GLYPH_SIZE "glyph_size"
#define FUNC_ON_DRAW "on_draw"
#define FUNC_ON_RESIZE "on_resize"
#define FUNC_ON_LOAD "on_load"
#define FUNC_ON_UNLOAD "on_unload"
#define FUNC_SET_COLOR "set_color"
#define FUNC_SET_HIGH_FPS "set_high_fps"

static int close_window(lua_State* L) {
  if (!verify_arg_count(L, 0, __func__)) {
    return 0;
  }

  lua_getglobal(L, GLOBAL_WINDOW_PTR);
  procy_window_t* window = (procy_window_t*)lua_touserdata(L, -1);
  procy_close_window(window);
  return 0;
}

static int window_size(lua_State* L) {
  if (!verify_arg_count(L, 0, __func__)) {
    return 0;
  }

  lua_getglobal(L, GLOBAL_WINDOW_PTR);
  procy_window_t* window = (procy_window_t*)lua_touserdata(L, -1);
  int width = 0, height = 0;
  procy_get_window_size(window, &width, &height);
  lua_pushinteger(L, width);
  lua_pushinteger(L, height);
  return 2;
}

static int set_window_high_fps(lua_State* L) {
  if (!verify_arg_count(L, 1, __func__)) {
    return 0;
  }

  bool enabled = lua_toboolean(L, -1);
  lua_getglobal(L, GLOBAL_WINDOW_PTR);
  ((procy_window_t*)lua_touserdata(L, -1))->high_fps = enabled;
  return 0;
}

static int window_glyph_size(lua_State* L) {
  if (!verify_arg_count(L, 0, __func__)) {
    return 0;
  }

  lua_getglobal(L, GLOBAL_WINDOW_PTR);
  procy_window_t* window = (procy_window_t*)lua_touserdata(L, -1);
  int width = 0, height = 0;
  procy_get_glyph_size(window, &width, &height);
  lua_pushinteger(L, width);
  lua_pushinteger(L, height);
  return 2;
}

static int window_reload(lua_State* L) {
  // close the window and set the "reload" flag to true
  close_window(L);
  lua_getglobal(L, GLOBAL_ENV_PTR);
  script_env_t* env = (script_env_t*)lua_touserdata(L, -1);
  env->reload = true;

  return 0;
}

// called from the main window loop by way of script_env_t.on_draw
static void perform_draw(procy_state_t* const state, double seconds) {
  lua_State* L = ((script_env_t*)state->data)->L;
  lua_getglobal(L, TBL_WINDOW);

  if (lua_getfield(L, -1, FUNC_ON_DRAW) != LUA_TNONE && lua_isfunction(L, -1)) {
    lua_pushnumber(L, seconds);
    if (lua_pcall(L, 1, 0, 0) == LUA_ERRRUN) {
      log_error("Error calling %s.%s: %s", TBL_WINDOW, FUNC_ON_DRAW,
                lua_tostring(L, -1));
    }
<<<<<<< HEAD

    // check whether the window is in "high fps" mode and, if it is NOT, run the
    // Lua garbage collector
    lua_getglobal(L, GLOBAL_WINDOW_PTR);
    procy_window_t* window = (procy_window_t*)lua_touserdata(L, -1);
    if (!window->high_fps) {
      lua_gc(L, LUA_GCCOLLECT);
    }
=======
>>>>>>> 5a47373a6b26524ae1a6b8c48f72e9a3f7a16059
  }
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
    lua_gc(L, LUA_GCCOLLECT);
  }
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
    lua_gc(L, LUA_GCCOLLECT);
  }
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
    lua_gc(L, LUA_GCCOLLECT);
  }
}
<<<<<<< HEAD

static int set_window_color(lua_State* L) {
  if (!verify_arg_count(L, 1, __func__)) {
    return 0;
  }

=======

static int set_window_color(lua_State* L) {
  if (!verify_arg_count(L, 1, __func__)) {
    return 0;
  }

>>>>>>> 5a47373a6b26524ae1a6b8c48f72e9a3f7a16059
  procy_set_clear_color(get_color(L, 1));
  lua_pop(L, lua_gettop(L));
  return 0;
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

  lua_pushcfunction(L, window_reload);
  lua_setfield(L, -2, FUNC_RELOAD);

  lua_pushcfunction(L, set_window_color);
  lua_setfield(L, -2, FUNC_SET_COLOR);

  lua_pushcfunction(L, set_window_high_fps);
  lua_setfield(L, -2, FUNC_SET_HIGH_FPS);

  lua_setglobal(L, TBL_WINDOW);
}
