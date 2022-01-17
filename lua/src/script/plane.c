#include <lauxlib.h>
#include <log.h>
#include <lua.h>
#include <stdlib.h>

#include "script/environment.h"

#define TBL_PLANE "plane"
#define TBL_PLANE_META "procyon_plane_meta"

#define FUNC_PLANE_FROM "from"
#define FUNC_PLANE_FILL "fill"
#define FUNC_PLANE_FOREACH "foreach"
#define FUNC_PLANE_AT "at"
#define FUNC_PLANE_SET "set"
#define FUNC_PLANE_SUB "sub"
#define FUNC_PLANE_MASK4 "mask4"
#define FUNC_PLANE_MASK8 "mask8"
#define FIELD_PLANE_WIDTH "width"
#define FIELD_PLANE_HEIGHT "height"
#define FIELD_PLANE_PTR "ptr"

static int plane_sub(lua_State *L);

static int plane_destroy_buffer(lua_State *L) {
  lua_getfield(L, 1, FIELD_PLANE_PTR);
  if (!lua_isnoneornil(L, -1)) {
    int *buffer = (int *)lua_touserdata(L, -1);
    if (buffer != NULL) {
      free(buffer);
    }
  }

  return 0;
}

static int try_get(int x, int y, int width, int height, int *const buffer) {
  return (x >= 0 && x < width && y >= 0 && y < height) ? buffer[y * width + x]
                                                       : 0;
}

static bool apply_func_to_plane(lua_State *L, int func_index, int width,
                                int height, int *const buffer) {
  int initial_top = lua_gettop(L);
  for (size_t i = 0; i < (size_t)(width * height); ++i) {
    // push a copy of the function to be called, since lua_pcall will pop it
    // after it's completed
    lua_pushvalue(L, func_index);

    // push (x, y, current) values
    lua_pushinteger(L, (int)(i % width));
    lua_pushinteger(L, (int)(i / width));
    lua_pushinteger(L, (int)buffer[i]);

    if (lua_pcall(L, 3, 1, 0) != LUA_OK) {
      LOG_SCRIPT_ERROR(L, "%s", lua_tostring(L, -1));
      return false;
    }

    // if any values were returned, store the first in the buffer
    if (lua_gettop(L) > initial_top && !lua_isnoneornil(L, initial_top + 1)) {
      buffer[i] = luaL_optinteger(L, initial_top + 1, 0);
      lua_pop(L, lua_gettop(L) - initial_top);
    }
  }

  return true;
}

static int *get_plane_buffer(lua_State *L, int index, int *width, int *height) {
  if (width != NULL) {
    lua_getfield(L, index, FIELD_PLANE_WIDTH);
    *width = luaL_optinteger(L, -1, -1);
    lua_pop(L, 1);
  }

  if (height != NULL) {
    lua_getfield(L, index, FIELD_PLANE_HEIGHT);
    *height = luaL_optinteger(L, -1, -1);
    lua_pop(L, 1);
  }

  lua_getfield(L, index, FIELD_PLANE_PTR);
  if (lua_isnoneornil(L, -1)) {
    return NULL;
  }

  int *buffer = (int *)lua_touserdata(L, -1);
  lua_pop(L, 1);

  return buffer;
}

static int plane_at(lua_State *L) {
  lua_settop(L, 3);

  int x = (int)luaL_checkinteger(L, 2);
  int y = (int)luaL_checkinteger(L, 3);

  int width;
  int height;
  int *buffer = get_plane_buffer(L, 1, &width, &height);

  lua_pushinteger(L, try_get(x, y, width, height, buffer));

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
  int *buffer = get_plane_buffer(L, 1, &width, &height);
  size_t buffer_len = width * height;

  if (lua_isnumber(L, 2)) {
    int value = lua_tointeger(L, 2);
    memset(buffer, value, buffer_len);
  } else if (lua_isfunction(L, 2)) {
    apply_func_to_plane(L, 2, width, height, buffer);
  }

  return 0;
}

// modes:
//
// myplane:set(x, y, 20)
static int plane_set(lua_State *L) {
  lua_settop(L, 4);

  int x = luaL_checkinteger(L, 2);
  int y = luaL_checkinteger(L, 3);

  int width;
  int height;
  int *buffer = get_plane_buffer(L, 1, &width, &height);
  if (buffer == NULL) {
    LOG_SCRIPT_ERROR(L, "Invalid plane buffer at %s", lua_tostring(L, 1));
    return 0;
  }

  if (x < 0 || x > width || y < 0 || y > height) {
    LOG_SCRIPT_ERROR(L, "Plane index (%d, %d) is out-of-bounds", x, y);
    return 0;
  }

  buffer[y * width + x] = lua_tointeger(L, 4);

  return 0;
}

static int *push_new_plane(int width, int height, size_t *len, lua_State *L) {
  // ensure dimensions are sane
  if (width <= 0 || height <= 0) {
    LOG_SCRIPT_ERROR(L, "Invalid plane dimensions (%d, %d)", width, height);
    return NULL;
  }

  // attempt to allocate a zeroed-out buffer using those dimensions
  size_t buffer_len = (size_t)width * (size_t)height;
  int *buffer = calloc(buffer_len, sizeof(int));
  if (buffer == NULL) {
    // if something went wrong, log a stack trace and bail
    LOG_SCRIPT_ERROR(L,
                     "Failed to allocate memory for plane buffer of size %zu",
                     buffer_len);
    return NULL;
  }

  if (len != NULL) {
    *len = buffer_len;
  }

  // set up return table; includes some metadata and a pointer to the C buffer
  lua_newtable(L);
  luaL_setmetatable(L, TBL_PLANE_META);

  lua_pushinteger(L, width);
  lua_setfield(L, -2, FIELD_PLANE_WIDTH);

  lua_pushinteger(L, height);
  lua_setfield(L, -2, FIELD_PLANE_HEIGHT);

  lua_pushlightuserdata(L, (void *)buffer);
  lua_setfield(L, -2, FIELD_PLANE_PTR);

  return buffer;
}

// plane:sub(x, y, w, h)
static int plane_sub(lua_State *L) {
  lua_settop(L, 5);

  int source_x = (int)luaL_checkinteger(L, 2);
  int source_y = (int)luaL_checkinteger(L, 3);
  int target_width = (int)luaL_checkinteger(L, 4);
  int target_height = (int)luaL_checkinteger(L, 5);

  int source_width = 0;
  int source_height = 0;
  int *source_buffer = get_plane_buffer(L, 1, &source_width, &source_height);

  if (source_buffer == NULL) {
    return 0;
  }

  int *target_buffer = push_new_plane(target_width, target_height, NULL, L);

  if (target_buffer == NULL) {
    return 0;
  }

  for (int x = 0; x < target_width; ++x) {
    for (int y = 0; y < target_height; ++y) {
      target_buffer[y * target_width + x] =
          try_get(source_x + x, source_y + y, source_width, source_height,
                  source_buffer);
    }
  }

  return 1;
}

// plane:mask4(x, y, filter)
//
//       1
//     2 c 8
//       4
static int plane_mask_4_directions(lua_State *L) {
  lua_settop(L, 4);

  int x = (int)luaL_checkinteger(L, 2);
  int y = (int)luaL_checkinteger(L, 3);
  int filter = (int)luaL_optinteger(L, 4, 0xFFFFFFFF);

  int width = 0;
  int height = 0;
  int *buffer = get_plane_buffer(L, 1, &width, &height);

  int mask = 0;
  mask |= (try_get(x, y - 1, width, height, buffer) & filter) ? 1 : 0;
  mask |= (try_get(x - 1, y, width, height, buffer) & filter) ? 1 << 1 : 0;
  mask |= (try_get(x, y + 1, width, height, buffer) & filter) ? 1 << 2 : 0;
  mask |= (try_get(x + 1, y, width, height, buffer) & filter) ? 1 << 3 : 0;

  return mask;
}

// plane:mask8(x, y, filter)
//
//     1   128  64
//     2   c    32
//     4   8    16
static int plane_mask_8_directions(lua_State *L) {
  lua_settop(L, 4);

  int x = (int)luaL_checkinteger(L, 2);
  int y = (int)luaL_checkinteger(L, 3);
  int filter = (int)luaL_optinteger(L, 4, 0xFFFFFFFF);

  int width = 0;
  int height = 0;
  int *buffer = get_plane_buffer(L, 1, &width, &height);

  int mask = 0;

  // clang-format off
  
  mask |= (try_get(x - 1, y - 1, width, height, buffer) & filter)   ? 1 : 0;
  mask |= (try_get(x - 1, y, width, height, buffer) & filter)       ? 1 << 1 : 0;
  mask |= (try_get(x - 1, y + 1, width, height, buffer) & filter)   ? 1 << 2 : 0;
  mask |= (try_get(x, y + 1, width, height, buffer) & filter)       ? 1 << 3 : 0;
  mask |= (try_get(x + 1, y + 1, width, height, buffer) & filter)   ? 1 << 4 : 0;
  mask |= (try_get(x + 1, y, width, height, buffer) & filter)       ? 1 << 5 : 0;
  mask |= (try_get(x + 1, y - 1, width, height, buffer) & filter)   ? 1 << 6 : 0;
  mask |= (try_get(x, y - 1, width, height, buffer) & filter)       ? 1 << 7 : 0;

  // clang-format on

  return mask;
}

// modes:
//
// plane.from(w, h, 4)
// plane.from(w, h, function(x, y) return x + y end)
static int plane_from(lua_State *L) {
  // fetch plane dimensions from first two arguments
  int width = lua_tointeger(L, 1);
  int height = lua_tointeger(L, 2);

  size_t buffer_len;
  int *buffer = push_new_plane(width, height, &buffer_len, L);
  if (buffer == NULL) {
    return 0;
  }

  if (lua_isnumber(L, 3)) {
    memset(buffer, lua_tointeger(L, 3), buffer_len);
  } else if (lua_isfunction(L, 3)) {
    apply_func_to_plane(L, 3, width, height, buffer);
  }

  return 1;
}

void add_plane(lua_State *L) {
  // initialize library table
  luaL_Reg create_methods[] = {{FUNC_PLANE_FROM, plane_from}, {NULL, NULL}};
  luaL_newlib(L, create_methods);
  lua_setglobal(L, TBL_PLANE);

  // initialize metatable
  luaL_Reg metamethods[] = {{"__gc", plane_destroy_buffer}, {NULL, NULL}};
  luaL_newlib(L, metamethods);
  luaL_Reg index_methods[] = {{FUNC_PLANE_FROM, plane_from},
                              {FUNC_PLANE_AT, plane_at},
                              {FUNC_PLANE_SET, plane_set},
                              {FUNC_PLANE_FILL, plane_fill},
                              {FUNC_PLANE_FOREACH, plane_fill},
                              {FUNC_PLANE_SUB, plane_sub},
                              {FUNC_PLANE_MASK4, plane_mask_4_directions},
                              {FUNC_PLANE_MASK8, plane_mask_8_directions},
                              {NULL, NULL}};
  luaL_newlib(L, index_methods);
  lua_setfield(L, -2, "__index");
  lua_setfield(L, LUA_REGISTRYINDEX, TBL_PLANE_META);
}
