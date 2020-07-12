#include "window.h"

#include <log.h>
#include <lua.h>

#include "script.h"
#include "script/environment.h"

#define TBL_WINDOW "window"

#define FUNC_CLOSE "close"
#define FUNC_ON_DRAW "on_draw"
#define FUNC_ON_RESIZE "on_resize"
#define FUNC_ON_LOAD "on_load"
#define FUNC_ON_UNLOAD "on_unload"

static int close_window(lua_State* L) {
  if (!verify_arg_count(L, 0, __func__)) {
    return 0;
  }

  lua_getglobal(L, GLOBAL_WINDOW_PTR);
  window_t* window = (window_t*)lua_touserdata(L, -1);
  window->quitting = true;
  lua_pop(L, 1);
  return 0;
}

// called from the main window loop by way of script_env_t.on_draw
static void perform_draw(script_env_t* env) {
  lua_State* L = env->L;
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

static void handle_window_resized(script_env_t* env, int w, int h) {
  lua_State* L = env->L;
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

static void handle_window_loaded(script_env_t* env) {
  lua_State* L = env->L;
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

static void handle_window_unloaded(script_env_t* env) {
  lua_State* L = env->L;
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

  env->on_draw = perform_draw;
  env->on_resized = handle_window_resized;
  env->on_load = handle_window_loaded;
  env->on_unload = handle_window_unloaded;

  lua_pushcfunction(L, close_window);
  lua_setfield(L, -2, FUNC_CLOSE);

  lua_setglobal(L, TBL_WINDOW);
}
