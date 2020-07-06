#include "window.h"

#include "script/environment.h"

#define TBL_WINDOW "window"

#define FUNC_CLOSE "close"
#define FUNC_REFRESH "refresh"
#define FUNC_SET_TILE_SIZE "set_tile_size"

static int close_window(lua_State* L) {
  if (!verify_arg_count(L, 0, __func__)) {
    return 0;
  }

  lua_getglobal(L, GLOBAL_WINDOW_PTR);
  window_t* window = (window_t*)lua_touserdata(L, -1);
  lua_pop(L, 1);
  window->quitting = true;
  return 0;
}

static int set_tile_size(lua_State* L) {
  if (!verify_arg_count(L, 2, __func__)) {
    return 0;
  }

  lua_getglobal(L, GLOBAL_WINDOW_PTR);
  window_t* window = (window_t*)lua_touserdata(L, -1);
  lua_pop(L, 1);

  window->tile_bounds.width = lua_tointeger(L, -1);
  window->tile_bounds.height = lua_tointeger(L, -2);
  lua_pop(L, 2);

  // refresh the view to re-draw tiles
  set_state_dirty(window);
  return 0;
}

static int refresh_window(lua_State* L) {
  if (!verify_arg_count(L, 0, __func__)) {
    return 0;
  }

  lua_getglobal(L, GLOBAL_WINDOW_PTR);
  set_state_dirty((window_t*)lua_touserdata(L, -1));
  lua_pop(L, 1);
  return 0;
}

void add_window(lua_State* L, script_env_t* env) {
  lua_newtable(L);

  lua_pushcfunction(L, close_window);
  lua_setfield(L, -2, FUNC_CLOSE);

  lua_pushcfunction(L, refresh_window);
  lua_setfield(L, -2, FUNC_REFRESH);

  lua_pushcfunction(L, set_tile_size);
  lua_setfield(L, -2, FUNC_SET_TILE_SIZE);

  lua_setglobal(L, TBL_WINDOW);
}
