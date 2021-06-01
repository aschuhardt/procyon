#include <log.h>
#include <lua.h>

#include "procyon.h"
#include "script.h"
#include "script/environment.h"

#define TBL_INPUT "input"

#define FUNC_EVENTS_KEYPRESS "on_key_pressed"
#define FUNC_EVENTS_KEYRELEASE "on_key_released"
#define FUNC_EVENTS_CHAR "on_char_entered"

#define FIELD_KEY_VALUE "value"
#define FIELD_KEY_NAME "name"
#define FIELD_KEY_CTRL "ctrl"
#define FIELD_KEY_SHIFT "shift"
#define FIELD_KEY_ALT "alt"

#define CHAR_MAX_CODEPOINT 255

static void push_key_arg(lua_State *L, procy_key_info_t *key, bool shift,
                         bool ctrl, bool alt) {
  lua_newtable(L);

  lua_pushinteger(L, key->value);
  lua_setfield(L, -2, FIELD_KEY_VALUE);

  lua_pushstring(L, key->name);
  lua_setfield(L, -2, FIELD_KEY_NAME);

  lua_pushboolean(L, shift);
  lua_setfield(L, -2, FIELD_KEY_SHIFT);

  lua_pushboolean(L, ctrl);
  lua_setfield(L, -2, FIELD_KEY_CTRL);

  lua_pushboolean(L, alt);
  lua_setfield(L, -2, FIELD_KEY_ALT);
}

static void handle_key_pressed(procy_state_t *const state, procy_key_info_t key,
                               bool shift, bool ctrl, bool alt) {
  lua_State *L = ((script_env_t *)state->data)->L;

  lua_getglobal(L, TBL_INPUT);
  lua_getfield(L, -1, FUNC_EVENTS_KEYPRESS);
  if (lua_isfunction(L, -1)) {
    push_key_arg(L, &key, shift, ctrl, alt);
    if (lua_pcall(L, 1, 0, 0) == LUA_ERRRUN) {
      LOG_SCRIPT_ERROR(L, "Error calling %s.%s: %s", TBL_INPUT,
                       FUNC_EVENTS_KEYPRESS, lua_tostring(L, -1));
    }
  }

  lua_pop(L, lua_gettop(L));
}

static void handle_key_released(procy_state_t *const state,
                                procy_key_info_t key, bool shift, bool ctrl,
                                bool alt) {
  lua_State *L = ((script_env_t *)state->data)->L;

  lua_getglobal(L, TBL_INPUT);
  lua_getfield(L, -1, FUNC_EVENTS_KEYRELEASE);
  if (lua_isfunction(L, -1)) {
    push_key_arg(L, &key, shift, ctrl, alt);
    if (lua_pcall(L, 1, 0, 0) == LUA_ERRRUN) {
      LOG_SCRIPT_ERROR(L, "Error calling %s.%s: %s", TBL_INPUT,
                       FUNC_EVENTS_KEYPRESS, lua_tostring(L, -1));
    }
  }

  lua_pop(L, lua_gettop(L));
}

static void handle_char_entered(procy_state_t *const state,
                                unsigned int codepoint) {
  // for the time being we're using a font that only supports so-called
  // "extended ASCII", up to value 255
  if (codepoint > CHAR_MAX_CODEPOINT) {
    return;
  }

  lua_State *L = ((script_env_t *)state->data)->L;

  lua_getglobal(L, TBL_INPUT);
  lua_getfield(L, -1, FUNC_EVENTS_CHAR);
  if (lua_isfunction(L, -1)) {
    char buffer[2] = {(char)codepoint,
                      '\0'};  // pass the truncated codepoint, followed by a
                              // null-terminator as a two-byte string
    lua_pushstring(L, &buffer[0]);
    if (lua_pcall(L, 1, 0, 0) == LUA_ERRRUN) {
      LOG_SCRIPT_ERROR(L, "Error calling %s.%s: %s", TBL_INPUT,
                       FUNC_EVENTS_CHAR, lua_tostring(L, -1));
    }
  }

  lua_pop(L, lua_gettop(L));
}

static void add_event_handler_table(lua_State *L) {
  lua_newtable(L);
  lua_setglobal(L, TBL_INPUT);
}

static void add_keys(lua_State *L) {
  size_t keys_count = 0;
  procy_key_info_t *keys = NULL;
  procy_get_keys(&keys, &keys_count);

  // store named key values
  for (int i = 0; i < keys_count; ++i) {
    procy_key_info_t key = keys[i];

    lua_pushinteger(L, key.value);
    lua_setglobal(L, key.name);
  }

  free(keys);
}

void add_input(lua_State *L, script_env_t *env) {
  add_event_handler_table(L);
  add_keys(L);
  env->state->on_key_pressed = handle_key_pressed;
  env->state->on_key_released = handle_key_released;
  env->state->on_char_entered = handle_char_entered;
}
