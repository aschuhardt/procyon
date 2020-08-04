#include <log.h>
#include <lua.h>
#include <string.h>

#include "procyon.h"
#include "script.h"
#include "script/environment.h"

#define TBL_LOG "log"

#define FUNC_LOG_INFO "info"
#define FUNC_LOG_ERROR "error"
#define FUNC_LOG_WARN "warn"
#define FUNC_LOG_DEBUG "debug"

static int write_log_info(lua_State* L) {
  if (verify_arg_count(L, 1, __func__)) {
    log_info("%s", lua_tostring(L, 1));
  }

  return 0;
}

static int write_log_error(lua_State* L) {
  if (verify_arg_count(L, 1, __func__)) {
    log_error("%s", lua_tostring(L, 1));
  }

  return 0;
}

static int write_log_warn(lua_State* L) {
  if (verify_arg_count(L, 1, __func__)) {
    log_warn("%s", lua_tostring(L, 1));
  }

  return 0;
}

static int write_log_debug(lua_State* L) {
  if (verify_arg_count(L, 1, __func__)) {
    log_debug("%s", lua_tostring(L, 1));
  }

  return 0;
}

static void add_logging(lua_State* L) {
  lua_newtable(L);

  lua_pushcfunction(L, write_log_info);
  lua_setfield(L, -2, FUNC_LOG_INFO);

  lua_pushcfunction(L, write_log_error);
  lua_setfield(L, -2, FUNC_LOG_ERROR);

  lua_pushcfunction(L, write_log_warn);
  lua_setfield(L, -2, FUNC_LOG_WARN);

  lua_pushcfunction(L, write_log_debug);
  lua_setfield(L, -2, FUNC_LOG_DEBUG);

  lua_setglobal(L, TBL_LOG);
}

void add_utilities(lua_State* L) { add_logging(L); }
