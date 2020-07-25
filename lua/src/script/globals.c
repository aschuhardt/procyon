#include <lua.h>

#include "script.h"
#include "script/environment.h"

void add_globals(lua_State* L, script_env_t* env) {
  typedef struct {
    void* data;
    const char* name;
  } entry_t;

  const entry_t entries[] = {{env, GLOBAL_ENV_PTR},
                             {env->window, GLOBAL_WINDOW_PTR}};

  for (int i = 0; i < sizeof(entries) / sizeof(entry_t); ++i) {
    lua_pushlightuserdata(L, entries[i].data);
    lua_setglobal(L, entries[i].name);
  }
}
