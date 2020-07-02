#include <log.h>
#include <string.h>

#include "script/environment.h"
#include "window.h"

#define TBL_DRAWING "draw"

#define FUNC_DRAWSTRING "string"

#define MAX_STRING_DRAW_LENGTH 1024

int draw_string(lua_State* L) {
  if (!verify_arg_count(L, 3, __func__)) {
    return 0;
  }

  const char* contents = lua_tostring(L, -1);
  int y = lua_tointeger(L, -2);
  int x = lua_tointeger(L, -3);

  lua_pop(L, 3);

  lua_getglobal(L, GLOBAL_WINDOW_PTR);
  window_t* window = (window_t*)lua_touserdata(L, -1);

  size_t len = strnlen(contents, MAX_STRING_DRAW_LENGTH);
  for (int i = 0; i < len; ++i) {
    glyph_t g = {x + i, y, (unsigned char)contents[i]};
    add_glyph_to_buffer(window, g);
  }

  return 0;
}

void add_drawing(lua_State* L, script_env_t* env) {
  lua_newtable(L);

  lua_pushcfunction(L, draw_string);
  lua_setfield(L, -2, FUNC_DRAWSTRING);

  lua_setglobal(L, TBL_DRAWING);
}
