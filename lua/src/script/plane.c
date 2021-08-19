#include <lauxlib.h>
#include <log.h>
#include <lua.h>
#include <stdlib.h>

#include "script/environment.h"

#define TBL_PLANE "plane"
#define TBL_PLANE_META "procyon_plane_meta"

#define FUNC_PLANE_FROM "from"
#define FUNC_PLANE_FILL "fill"
#define FUNC_PLANE_AT "at"
#define FUNC_PLANE_SET "set"
#define FIELD_PLANE_WIDTH "width"
#define FIELD_PLANE_HEIGHT "height"
#define FIELD_PLANE_PTR "ptr"

static int plane_destroy_buffer(lua_State *L) {
  lua_getfield(L, 1, FIELD_PLANE_PTR);
  if (!lua_isnoneornil(L, -1)) {
    unsigned char *buffer = (unsigned char *)lua_touserdata(L, -1);
    if (buffer != NULL) {
      free(buffer);
    }
  }

  return 0;
}

static bool apply_func_to_plane(lua_State *L, int func_index, int width,
                                int height, unsigned char *const buffer) {
  for (int x = 0; x < width; ++x) {
    for (int y = 0; y < height; ++y) {
      // push a copy of the function to be called, since lua_pcall will pop it
      // after it's completed
      lua_pushvalue(L, func_index);

      // push the x, y values
      lua_pushinteger(L, x);
      lua_pushinteger(L, y);

      if (lua_pcall(L, 2, 1, 0) != LUA_OK) {
        LOG_SCRIPT_ERROR(L, "%s", lua_tostring(L, -1));
        return false;
      }

      // verify that we received a value return value
      buffer[y * width + x] = luaL_optinteger(L, -1, 0) % (UCHAR_MAX + 1);

      // clear the return value off the stack
      lua_pop(L, 1);
    }
  }

  return true;
}

static unsigned char *get_plane_buffer(lua_State *L, int index, int *width,
                                       int *height) {
  if (width != NULL) {
    lua_getfield(L, index, FIELD_PLANE_WIDTH);
    *width = lua_isinteger(L, -1) ? (int)lua_tointeger(L, -1) : -1;
    lua_pop(L, 1);
  }

  if (height != NULL) {
    lua_getfield(L, index, FIELD_PLANE_HEIGHT);
    *height = lua_isinteger(L, -1) ? (int)lua_tointeger(L, -1) : -1;
    lua_pop(L, 1);
  }

  lua_getfield(L, index, FIELD_PLANE_PTR);
  if (lua_isnoneornil(L, -1)) {
    return NULL;
  }

  unsigned char *buffer = (unsigned char *)lua_touserdata(L, -1);
  lua_pop(L, 1);

  return buffer;
}

static int plane_at(lua_State *L) {
  lua_settop(L, 3);

  int x = (int)luaL_checkinteger(L, 2);
  int y = (int)luaL_checkinteger(L, 3);

  int width;
  int height;
  unsigned char *buffer = get_plane_buffer(L, 1, &width, &height);
  if (x < 0 || x > width || y < 0 || y > height) {
    LOG_SCRIPT_ERROR(L, "Plane index (%d, %d) is out-of-bounds", x, y);
    return 0;
  }

  lua_pushinteger(L, buffer[y * width + x]);

  return 1;
}

// modes:
//
// myplane:fill(3)
// myplane:fill(function(x, y, cur) return 2 + 1 end)
static int plane_fill(lua_State *L) {
  lua_settop(L, 2);

  int width;
  int height;
  unsigned char *buffer = get_plane_buffer(L, 1, &width, &height);
  size_t buffer_len = width * height;

  if (lua_isinteger(L, 2)) {
    int value = lua_tointeger(L, 2) % (UCHAR_MAX + 1);
    memset(buffer, value, buffer_len);
  } else if (lua_isfunction(L, 2)) {
    for (long long i = 0; i < buffer_len; ++i) {
      lua_pushvalue(L, 2);
      lua_pushinteger(L, i / width);
      lua_pushinteger(L, i % width);
      lua_pushinteger(L, (int)buffer[i]);
      if (lua_pcall(L, 3, LUA_MULTRET, 0) != LUA_OK) {
        LOG_SCRIPT_ERROR(L, "Plane fill function failed: %s",
                         lua_tostring(L, -1));
        break;
      }

      if (lua_isinteger(L, -1)) {
        buffer[i] = luaL_checkinteger(L, -1) % (UCHAR_MAX + 1);
      }

      lua_pop(L, 1);  // pop return value
    }
  }

  return 0;
}

// modes:
//
// myplane:set(x, y, 20)
// myplane:set(x, y, "hello")
static int plane_set(lua_State *L) {
  lua_settop(L, 4);

  int x = (int)luaL_checkinteger(L, 2);
  int y = (int)luaL_checkinteger(L, 3);

  int width;
  int height;
  unsigned char *buffer = get_plane_buffer(L, 1, &width, &height);
  if (x < 0 || x > width || y < 0 || y > height) {
    LOG_SCRIPT_ERROR(L, "Plane index (%d, %d) is out-of-bounds", x, y);
    return 0;
  }

  unsigned char *cursor = &buffer[y * width + x];
  if (lua_isinteger(L, 4)) {
    *cursor = (unsigned char)(lua_tointeger(L, 4) % (UCHAR_MAX + 1));
  } else if (lua_isstring(L, 4)) {
    // store each byte in the string consecutively
    size_t len = 0;
    const char *text = lua_tolstring(L, 4, &len);
    for (size_t i = 0; i < len && cursor - buffer < width * height; ++i) {
      *(cursor++) = (unsigned char)text[i];
    }
  }

  return 0;
}

// modes:
//
// plane.from(w, h, 4)
// plane.from(w, h, function(x, y) return x + y end)
static int plane_from(lua_State *L) {
  // fetch plane dimensions from first two arguments
  int width = lua_tointeger(L, 1);
  int height = lua_tointeger(L, 2);

  // ensure dimensions are sane
  if (width <= 0 || height <= 0) {
    LOG_SCRIPT_ERROR(L, "Invalid plane dimensions (%d, %d)", width, height);
    return 0;
  }

  // attempt to allocate a zeroed-out buffer using those dimensions
  size_t buffer_len = (size_t)width * (size_t)height;
  unsigned char *buffer = calloc(buffer_len, sizeof(unsigned char));
  if (buffer == NULL) {
    // if something went wrong, log a stack trace and bail
    LOG_SCRIPT_ERROR(L,
                     "Failed to allocate memory for plane buffer of size %zu",
                     buffer_len);
    return 0;
  }

  if (lua_gettop(L) == 3) {
    if (lua_isinteger(L, 3)) {
      memset(buffer, lua_tointeger(L, 3), buffer_len);
    } else if (lua_isfunction(L, 3)) {
      apply_func_to_plane(L, 3, width, height, buffer);
    }
  }

  lua_pop(L, lua_gettop(L));

  // set up return table; includes some metadata and a pointer to the C buffer
  lua_newtable(L);
  luaL_setmetatable(L, TBL_PLANE_META);

  lua_pushinteger(L, width);
  lua_setfield(L, -2, FIELD_PLANE_WIDTH);

  lua_pushinteger(L, height);
  lua_setfield(L, -2, FIELD_PLANE_HEIGHT);

  lua_pushlightuserdata(L, (void *)buffer);
  lua_setfield(L, -2, FIELD_PLANE_PTR);

  lua_pushcfunction(L, plane_at);
  lua_setfield(L, -2, FUNC_PLANE_AT);

  lua_pushcfunction(L, plane_set);
  lua_setfield(L, -2, FUNC_PLANE_SET);

  lua_pushcfunction(L, plane_fill);
  lua_setfield(L, -2, FUNC_PLANE_FILL);

  return 1;
}

void add_plane(lua_State *L) {
  // initialize library table
  luaL_Reg methods[] = {{FUNC_PLANE_FROM, plane_from},
                        {FUNC_PLANE_AT, plane_at},
                        {FUNC_PLANE_SET, plane_set},
                        {FUNC_PLANE_FILL, plane_fill},
                        {NULL, NULL}};
  luaL_newlib(L, methods);
  lua_setglobal(L, TBL_PLANE);

  // initialize metatable
  luaL_Reg metamethods[] = {{"__gc", plane_destroy_buffer}, {NULL, NULL}};
  luaL_newlib(L, metamethods);
  lua_setfield(L, LUA_REGISTRYINDEX, TBL_PLANE_META);
}
