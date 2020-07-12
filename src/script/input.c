#include <GLFW/glfw3.h>
#include <log.h>
#include <lua.h>

#include "script.h"
#include "script/environment.h"
#include "script/keys.h"
#include "window.h"

#define TBL_INPUT "input"
#define TBL_KEYS "keys"
#define FLD_KEY_NAME "name"
#define FLD_KEY_VALUE "value"

#define FUNC_EVENTS_KEYPRESS "key_pressed"
#define FUNC_EVENTS_KEYRELEASED "key_released"
#define FUNC_EVENTS_CHAR "char_entered"

#define CHAR_MAX_CODEPOINT 255

static inline lua_State* get_lua_env_from_glfwwindow(GLFWwindow* w) {
  return ((script_env_t*)((window_t*)glfwGetWindowUserPointer(w))->script_state)
      ->L;
}

static void glfw_key_callback(GLFWwindow* w, int key, int scancode, int action,
                              int mods) {
  lua_State* L = get_lua_env_from_glfwwindow(w);

  lua_getglobal(L, TBL_INPUT);
  if (lua_isnoneornil(L, -1)) {
    log_warn("The global %s table does not exist", TBL_INPUT);
    return;
  }

  // keep this in memory for logging purposes
  const char* func_name =
      action == GLFW_PRESS ? FUNC_EVENTS_KEYPRESS : FUNC_EVENTS_KEYRELEASED;

  lua_getfield(L, -1, func_name);
  if (lua_isfunction(L, -1)) {
    lua_pushinteger(L, key);
    if (lua_pcall(L, 1, 0, 0) == LUA_ERRRUN) {
      log_error("Error calling %s.%s: %s", TBL_INPUT, func_name,
                lua_tostring(L, -1));
    }
  }
}

static void glfw_char_callback(GLFWwindow* w, unsigned int codepoint) {
  // for the time being we're using a font that only supports so-called
  // "extended ASCII", up to value 255
  if (codepoint > CHAR_MAX_CODEPOINT) {
    return;
  }

  lua_State* L = get_lua_env_from_glfwwindow(w);

  lua_getglobal(L, TBL_INPUT);
  if (lua_isnoneornil(L, -1)) {
    log_warn("The global %s table does not exist", TBL_INPUT);
    return;
  }

  lua_getfield(L, -1, FUNC_EVENTS_CHAR);
  if (lua_isfunction(L, -1)) {
    char buffer[2] = {(char)codepoint,
                      '\0'};  // pass the truncated codepoint, followed by a
                              // null-terminator as a two-byte string
    lua_pushstring(L, &buffer[0]);
    if (lua_pcall(L, 1, 0, 0) == LUA_ERRRUN) {
      log_error("Error calling %s.%s: %s", TBL_INPUT, FUNC_EVENTS_CHAR,
                lua_tostring(L, -1));
    }
  }
}

static void add_event_handler_table(lua_State* L) {
  lua_newtable(L);
  lua_setglobal(L, TBL_INPUT);
}

static void add_keys(lua_State* L) {
  lua_newtable(L);

  // store named key values
  for (int i = 0; i < sizeof(KEYS) / sizeof(key_info_t); ++i) {
    key_info_t key = KEYS[i];

    // [name] = value
    lua_pushinteger(L, key.value);
    lua_setfield(L, -2, key.name);

    // [value] = { name, value }
    lua_newtable(L);
    lua_pushstring(L, key.name);
    lua_setfield(L, -2, FLD_KEY_NAME);
    lua_pushinteger(L, key.value);
    lua_setfield(L, -2, FLD_KEY_VALUE);
    lua_seti(L, -2, key.value);

    log_trace("Adding key value (%s, %d)", key.name, key.value);
  }

  lua_setglobal(L, TBL_KEYS);
}

void add_input(lua_State* L, script_env_t* env) {
  add_event_handler_table(L);
  add_keys(L);
  glfwSetKeyCallback((GLFWwindow*)env->window->glfw_win, glfw_key_callback);
  glfwSetCharCallback((GLFWwindow*)env->window->glfw_win, glfw_char_callback);
}
