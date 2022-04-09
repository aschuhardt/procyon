#include <log.h>
#include <lua.h>

#include "procyon.h"
#include "script.h"
#include "script/environment.h"

#define TBL_INPUT "input"

#define FUNC_EVENTS_KEYPRESS "on_key_pressed"
#define FUNC_EVENTS_KEYRELEASE "on_key_released"
#define FUNC_EVENTS_CHAR "on_char_entered"
#define FUNC_EVENTS_MOUSE_MOVED "on_mouse_moved"
#define FUNC_EVENTS_MOUSE_PRESS "on_mouse_pressed"
#define FUNC_EVENTS_MOUSE_RELEASE "on_mouse_released"

#define FIELD_KEY_VALUE "value"
#define FIELD_KEY_CTRL "ctrl"
#define FIELD_KEY_SHIFT "shift"
#define FIELD_KEY_ALT "alt"

#define FIELD_MOUSE_VALUE "value"
#define FIELD_MOUSE_CTRL "ctrl"
#define FIELD_MOUSE_SHIFT "shift"
#define FIELD_MOUSE_ALT "alt"

#define CHAR_MAX_CODEPOINT 255

static void push_key_arg(lua_State *L, procy_key_info_t *key, bool shift,
                         bool ctrl, bool alt) {
  lua_newtable(L);

  lua_pushinteger(L, key->value);
  lua_setfield(L, -2, FIELD_KEY_VALUE);

  lua_pushboolean(L, shift);
  lua_setfield(L, -2, FIELD_KEY_SHIFT);

  lua_pushboolean(L, ctrl);
  lua_setfield(L, -2, FIELD_KEY_CTRL);

  lua_pushboolean(L, alt);
  lua_setfield(L, -2, FIELD_KEY_ALT);
}

static void push_mouse_button_arg(lua_State *L, procy_mouse_button_t button,
                                  bool shift, bool ctrl, bool alt) {
  lua_newtable(L);

  lua_pushinteger(L, button);
  lua_setfield(L, -2, FIELD_MOUSE_VALUE);

  lua_pushboolean(L, shift);
  lua_setfield(L, -2, FIELD_MOUSE_SHIFT);

  lua_pushboolean(L, ctrl);
  lua_setfield(L, -2, FIELD_MOUSE_CTRL);

  lua_pushboolean(L, alt);
  lua_setfield(L, -2, FIELD_MOUSE_ALT);
}

static void key_pressed(procy_state_t *state, procy_key_info_t key, bool shift,
                        bool ctrl, bool alt) {
  lua_State *L = ((script_env_t *)state->data)->L;

  push_library_table(L);
  lua_getfield(L, -1, TBL_INPUT);
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

static void key_released(procy_state_t *state, procy_key_info_t key, bool shift,
                         bool ctrl, bool alt) {
  lua_State *L = ((script_env_t *)state->data)->L;

  push_library_table(L);
  lua_getfield(L, -1, TBL_INPUT);
  lua_getfield(L, -1, FUNC_EVENTS_KEYRELEASE);
  if (lua_isfunction(L, -1)) {
    push_key_arg(L, &key, shift, ctrl, alt);
    if (lua_pcall(L, 1, 0, 0) == LUA_ERRRUN) {
      LOG_SCRIPT_ERROR(L, "Error calling %s.%s: %s", TBL_INPUT,
                       FUNC_EVENTS_KEYRELEASE, lua_tostring(L, -1));
    }
  }

  lua_pop(L, lua_gettop(L));
}

static void char_entered(procy_state_t *state, unsigned int codepoint) {
  // for the time being we're using a font that only supports so-called
  // "extended ASCII", up to value 255
  if (codepoint > CHAR_MAX_CODEPOINT) {
    return;
  }

  lua_State *L = ((script_env_t *)state->data)->L;

  push_library_table(L);
  lua_getfield(L, -1, TBL_INPUT);
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

static void mouse_moved(procy_state_t *state, double x, double y) {
  lua_State *L = ((script_env_t *)state->data)->L;

  push_library_table(L);
  lua_getfield(L, -1, TBL_INPUT);
  lua_getfield(L, -1, FUNC_EVENTS_MOUSE_MOVED);
  if (lua_isfunction(L, -1)) {
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    if (lua_pcall(L, 2, 0, 0) == LUA_ERRRUN) {
      LOG_SCRIPT_ERROR(L, "Error calling %s.%s: %s", TBL_INPUT,
                       FUNC_EVENTS_MOUSE_MOVED, lua_tostring(L, -1));
    }
  }

  lua_pop(L, lua_gettop(L));
}

static void mouse_released(procy_state_t *state, procy_mouse_button_t button,
                           bool shift, bool ctrl, bool alt) {
  lua_State *L = ((script_env_t *)state->data)->L;

  push_library_table(L);
  lua_getfield(L, -1, TBL_INPUT);
  lua_getfield(L, -1, FUNC_EVENTS_MOUSE_RELEASE);
  if (lua_isfunction(L, -1)) {
    push_mouse_button_arg(L, button, shift, ctrl, alt);
    if (lua_pcall(L, 1, 0, 0) == LUA_ERRRUN) {
      LOG_SCRIPT_ERROR(L, "Error calling %s.%s: %s", TBL_INPUT,
                       FUNC_EVENTS_MOUSE_RELEASE, lua_tostring(L, -1));
    }
  }

  lua_pop(L, lua_gettop(L));
}

static void mouse_pressed(procy_state_t *state, procy_mouse_button_t button,
                          bool shift, bool ctrl, bool alt) {
  lua_State *L = ((script_env_t *)state->data)->L;

  push_library_table(L);
  lua_getfield(L, -1, TBL_INPUT);
  lua_getfield(L, -1, FUNC_EVENTS_MOUSE_PRESS);
  if (lua_isfunction(L, -1)) {
    push_mouse_button_arg(L, button, shift, ctrl, alt);
    if (lua_pcall(L, 1, 0, 0) == LUA_ERRRUN) {
      LOG_SCRIPT_ERROR(L, "Error calling %s.%s: %s", TBL_INPUT,
                       FUNC_EVENTS_MOUSE_PRESS, lua_tostring(L, -1));
    }
  }

  lua_pop(L, lua_gettop(L));
}

static void add_event_handler_table(lua_State *L) {
  lua_newtable(L);
  lua_setfield(L, 1, TBL_INPUT);
}

static void add_keys(lua_State *L) {
  size_t keys_count = 0;
  procy_key_info_t *keys = NULL;
  procy_get_keys(&keys, &keys_count);

  // store named key values
  for (int i = 0; i < keys_count; ++i) {
    procy_key_info_t key = keys[i];

    lua_pushinteger(L, key.value);
    lua_setfield(L, 1, key.name);
  }

  free(keys);
}

static void add_mouse_values(lua_State *L) {
  lua_pushinteger(L, MOUSE_BUTTON_0);
  lua_setfield(L, 1, "MOUSE_0");

  lua_pushinteger(L, MOUSE_BUTTON_1);
  lua_setfield(L, 1, "MOUSE_1");

  lua_pushinteger(L, MOUSE_BUTTON_2);
  lua_setfield(L, 1, "MOUSE_2");

  lua_pushinteger(L, MOUSE_BUTTON_3);
  lua_setfield(L, 1, "MOUSE_3");

  lua_pushinteger(L, MOUSE_BUTTON_4);
  lua_setfield(L, 1, "MOUSE_4");

  lua_pushinteger(L, MOUSE_BUTTON_5);
  lua_setfield(L, 1, "MOUSE_5");

  lua_pushinteger(L, MOUSE_BUTTON_6);
  lua_setfield(L, 1, "MOUSE_6");

  lua_pushinteger(L, MOUSE_BUTTON_7);
  lua_setfield(L, 1, "MOUSE_7");

  lua_pushinteger(L, MOUSE_BUTTON_8);
  lua_setfield(L, 1, "MOUSE_8");

  lua_pushinteger(L, MOUSE_BUTTON_LEFT);
  lua_setfield(L, 1, "MOUSE_LEFT");

  lua_pushinteger(L, MOUSE_BUTTON_RIGHT);
  lua_setfield(L, 1, "MOUSE_RIGHT");

  lua_pushinteger(L, MOUSE_BUTTON_MIDDLE);
  lua_setfield(L, 1, "MOUSE_MIDDLE");
}

void add_input(lua_State *L, script_env_t *env) {
  add_event_handler_table(L);
  add_keys(L);
  add_mouse_values(L);
  env->state->on_key_pressed = key_pressed;
  env->state->on_key_released = key_released;
  env->state->on_char_entered = char_entered;
  env->state->on_mouse_moved = mouse_moved;
  env->state->on_mouse_pressed = mouse_pressed;
  env->state->on_mouse_released = mouse_released;
}
