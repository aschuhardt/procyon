#include <lua.h>
#include <string.h>

#include "procyon.h"
#include "script/environment.h"

#define TBL_DRAWING "draw"

#define FUNC_DRAWSTRING "string"

static int draw_string(lua_State* L) {
  if (!verify_arg_count(L, 3, __func__)) {
    return 0;
  }

  const char* contents = lua_tostring(L, -1);
  int y = lua_tointeger(L, -2);
  int x = lua_tointeger(L, -3);

  lua_pop(L, 3);

  lua_getglobal(L, GLOBAL_WINDOW_PTR);
  procy_window_t* window = (procy_window_t*)lua_touserdata(L, -1);

  size_t length = strlen(contents);
  procy_draw_op_t op;
  for (size_t i = 0; i < length; i++) {
    op = procy_create_draw_op_string(x, y, window->glyph.width, contents, i);
    procy_append_draw_op(window, &op);
  }

  return 0;
}

void add_drawing(lua_State* L, script_env_t* env) {
  lua_newtable(L);

  lua_pushcfunction(L, draw_string);
  lua_setfield(L, -2, FUNC_DRAWSTRING);

  lua_setglobal(L, TBL_DRAWING);
}
