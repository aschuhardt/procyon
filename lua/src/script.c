#include "script.h"

#include <log.h>
#include <lua.h>
#include <lualib.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "procyon.h"
#include "script/environment.h"

static const char* get_lua_alloc_type_name(size_t t) {
  switch (t) {
    case LUA_TSTRING:
      return "String";
    case LUA_TTABLE:
      return "Table";
    case LUA_TFUNCTION:
      return "Function";
    case LUA_TUSERDATA:
      return "UserData";
    case LUA_TTHREAD:
      return "Thread";
    default:
      return "Other";
  }
}

typedef struct raw_file_reader_state_t {
  FILE* handle;
  char* buffer;
  char* path;  // for logging
} raw_file_reader_state_t;

static const char* raw_file_reader(lua_State* L, void* data, size_t* size) {
  (void)L;
  raw_file_reader_state_t* rdr_state = (raw_file_reader_state_t*)data;

  // the second time this is called, it needs to be cleaned up because it's
  // finished reading
  if (rdr_state->buffer != NULL) {
    free(rdr_state->buffer);
    *size = 0;
    return NULL;
  }

  // TODO: make this a proper buffered reader that doesn't load the entire file
  // at once

  // make sure that we can actually read the file
  if (rdr_state->handle == NULL) {
    log_error("Attempted to read file \"%s\" that was never opened",
              rdr_state->path);
    *size = 0;
    return NULL;
  }

  // read the file into memory and return a buffer containing its contents,
  // setting `size` to the size of the returned block

  FILE* handle = rdr_state->handle;

  fseek(handle, 0, SEEK_END);
  long length = ftell(handle);
  *size = length * sizeof(char);

  rdr_state->buffer = malloc(sizeof(char) * length);
  if (rdr_state->buffer == NULL) {
    log_error("Failed to allocate enough memory to hold file %s",
              rdr_state->path);
    *size = 0;
    return NULL;
  }

  fseek(handle, 0, SEEK_SET);
  fread(rdr_state->buffer, sizeof(char), length, handle);

  log_debug("Successfully read %zu bytes from file %s", length,
            rdr_state->path);

  return rdr_state->buffer;
}

static size_t get_file_dir_length(char* path) {
#ifdef WIN32
  char* last_segment = strrchr(path, '\\');
#else
  char* last_segment = strrchr(path, '/');
#endif

  if (last_segment == NULL) {
    return strnlen(path, PATH_MAX);
  }

  return last_segment - path;
}

static void add_package_path_from_entry(lua_State* L, char* path) {
  // get the "package" table
  lua_getglobal(L, "package");

  // push the value of the "path" record
  lua_getfield(L, -1, "path");

  // find and push the absolute directory of the provided file
  lua_pushstring(L, ";");
  lua_pushlstring(L, path, get_file_dir_length(path));
  lua_pushstring(L, "/?.lua");

  // concatenate the directory and the existing path value, then assign
  // those to package.path
  lua_concat(L, 4);
  log_debug("Module package path: %s", lua_tostring(L, -1));
  lua_setfield(L, -2, "path");

  // pop the "package" table off the stack to leave it clean
  lua_pop(L, 1);
}

static bool load_script_raw(lua_State* L, char* path) {
  log_debug("Attempting to load raw script file %s", path);

  FILE* handle = fopen(path, "rb");
  if (handle == NULL || ferror(handle)) {
    log_error("Could not open file %s", path);
    return false;
  }

  raw_file_reader_state_t rdr_state = {handle, NULL, path};
  int status = lua_load(L, raw_file_reader, &rdr_state, path, NULL);

  fclose(handle);

  switch (status) {
    case LUA_OK:
      log_info("Successfully loaded script file %s", path);
      add_package_path_from_entry(L, path);
      return true;
    case LUA_ERRSYNTAX:
      log_error("Syntax error in %s: %s", path, lua_tostring(L, -1));
      break;
    case LUA_ERRMEM:
      log_error("Out-of-memory error while loading %s: %s", path,
                lua_tostring(L, -1));
      break;
    default:
      log_error("Unknown error occurred while loading %s: %s", path,
                lua_tostring(L, -1));
      break;
  }

  return false;
}

static void* script_state_alloc(void* ud, void* ptr, size_t osize,
                                size_t nsize) {
  (void)ud;
  if (ptr != NULL) {
    if (nsize == 0) {
      free(ptr);
      return NULL;
    }
    return realloc(ptr, nsize);
  }
  return malloc(nsize);
}

script_env_t* create_script_env(procy_window_t* window, procy_state_t* state) {
  script_env_t* env = malloc(sizeof(script_env_t));
  env->L = lua_newstate(script_state_alloc, NULL);
  env->window = window;
  env->state = state;
  env->reload = false;
  return env;
}

void destroy_script_env(script_env_t* env) {
  if (env != NULL && env->L != NULL) {
    lua_close((lua_State*)env->L);
  }

  free(env);
}

bool load_scripts(script_env_t* env, char* path) {
  lua_State* L = (lua_State*)env->L;
  luaL_openlibs(L);

  if (load_script_raw(L, path)) {
    add_globals(L, env);
    add_utilities(L);
    add_input(L, env);
    add_window(L, env);
    add_drawing(L, env);

    if (lua_pcall(L, 0, LUA_MULTRET, 0) == LUA_ERRRUN) {
      log_error("%s", lua_tostring(L, -1));
      return false;
    }

    // script is loaded without errors
    return true;
  }

  // failed to load the script
  return false;
}
