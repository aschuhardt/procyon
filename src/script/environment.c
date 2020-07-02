#include "script/environment.h"

#include <log.h>

bool verify_arg_count(lua_State* L, int n, const char* name) {
  int arg_count = lua_gettop(L);
  if (arg_count < n) {
    lua_Debug debug;
    lua_getstack(L, 1, &debug);
    if (lua_getinfo(L, "Sl", &debug) != 0) {
      log_error(
          "%s:%d - invalid number of arguments passed to %s (expected %d, "
          "found %d)",
          debug.source, debug.currentline, name, n, arg_count);
    } else {
      log_error(
          "invalid number of arguments passed to %s (expected %d, found %d)",
          name, n, arg_count);
    }
    lua_error(L);
    return false;
  }
  return true;
}
