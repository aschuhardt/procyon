#include <lauxlib.h>
#include <log.h>
#include <lua.h>

#include "script/environment.h"

#define TBL_LOG "log"
#define TBL_SCRIPT "script"

#define FUNC_LOG_INFO "info"
#define FUNC_LOG_ERROR "error"
#define FUNC_LOG_WARN "warn"
#define FUNC_LOG_DEBUG "debug"

#define FUNC_SCRIPT_RUN "run"

static int write_log_info(lua_State *L) {
  log_info("%s", luaL_checkstring(L, 1));
  return 0;
}

static int write_log_error(lua_State *L) {
  LOG_SCRIPT_ERROR(L, "%s", luaL_checkstring(L, 1));
  return 0;
}

static int write_log_warn(lua_State *L) {
  LOG_SCRIPT_WARN(L, "%s", luaL_checkstring(L, 1));
  return 0;
}

static int write_log_debug(lua_State *L) {
  LOG_SCRIPT_DEBUG(L, "%s", luaL_checkstring(L, 1));
  return 0;
}

static int run_script(lua_State *L) {
  luaL_dostring(L, luaL_checkstring(L, 1));

  return 0;
}

static void add_logging(lua_State *L) {
  luaL_Reg methods[] = {{FUNC_LOG_INFO, write_log_info},
                        {FUNC_LOG_ERROR, write_log_error},
                        {FUNC_LOG_WARN, write_log_warn},
                        {FUNC_LOG_DEBUG, write_log_debug},
                        {NULL, NULL}};
  luaL_newlib(L, methods);
  lua_setglobal(L, TBL_LOG);
}

static void add_script(lua_State *L) {
  luaL_Reg methods[] = {{FUNC_SCRIPT_RUN, run_script}, {NULL, NULL}};
  luaL_newlib(L, methods);
  lua_setglobal(L, TBL_SCRIPT);
}

void add_utilities(lua_State *L) {
  add_logging(L);
  add_script(L);
}
