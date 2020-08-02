#include <log.h>
#include <lua.h>

#include "script/environment.h"

static int s_log_info(lua_State* L) {
  if (verify_arg_count(L, 1, __func__)) {
    log_info("%s", lua_tostring(L, 1));
  }

  return 0;
}

static int s_log_error(lua_State* L) {
  if (verify_arg_count(L, 1, __func__)) {
    log_error("%s", lua_tostring(L, 1));
  }

  return 0;
}

static int s_log_warn(lua_State* L) {
  if (verify_arg_count(L, 1, __func__)) {
    log_warn("%s", lua_tostring(L, 1));
  }

  return 0;
}

static int s_log_debug(lua_State* L) {
  if (verify_arg_count(L, 1, __func__)) {
    log_debug("%s", lua_tostring(L, 1));
  }

  return 0;
}

void add_utilities(lua_State* L) {
  lua_newtable(L);

  lua_pushcfunction(L, s_log_info);
  lua_setfield(L, -2, "info");

  lua_pushcfunction(L, s_log_error);
  lua_setfield(L, -2, "error");

  lua_pushcfunction(L, s_log_warn);
  lua_setfield(L, -2, "warn");

  lua_pushcfunction(L, s_log_debug);
  lua_setfield(L, -2, "debug");

  lua_setglobal(L, "log");
}
