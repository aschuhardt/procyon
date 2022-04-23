#include <base64.h>
#include <lauxlib.h>
#include <log.h>
#include <lua.h>
#include <simdcomp.h>
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
#define FUNC_PLANE_COPY "copy"
#define FUNC_PLANE_BLIT "blit"
#define FUNC_PLANE_ENCODE "encode"
#define FUNC_PLANE_DECODE "decode"
#define FUNC_PLANE_GETSIZE "get_size"
#define FIELD_PLANE_BUFFER "_buffer"
#define FIELD_PLANE_DATA "_data"
#define FIELD_PLANE_WIDTH "width"
#define FIELD_PLANE_HEIGHT "height"

typedef struct plane_t {
  int *buffer;
  int width;
  int height;
} plane_t;

static int plane_sub(lua_State *L);

static inline int try_get(int x, int y, int width, int height,
                          int *const buffer) {
  return (x >= 0 && x < width && y >= 0 && y < height) ? buffer[y * width + x]
                                                       : 0;
}

static inline int get(int x, int y, int width, int *const buffer) {
  return buffer[y * width + x];
}

static bool apply_func_to_buffer(lua_State *L, int func_index, int width,
                                 int height, int *const buffer) {
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

    // if any value was returned, store it in the buffer
    if (lua_isnumber(L, -1)) {
      buffer[i] = (int)lua_tointeger(L, -1);
    }

    lua_pop(L, 1);
  }

  return true;
}

static plane_t *get_plane(lua_State *L, int index) {
  lua_getfield(L, index, FIELD_PLANE_DATA);
  if (!lua_isuserdata(L, -1)) {
    LOG_SCRIPT_ERROR(L,
                     "Attempted to access a plane without a valid data field");
    return 0;
  }

  plane_t *plane = (plane_t *)lua_touserdata(L, -1);
  lua_pop(L, 1);

  if (plane == NULL) {
    LOG_SCRIPT_ERROR(L, "Missing plane data");
    return 0;
  }

  return plane;
}

static int plane_at(lua_State *L) {
  lua_settop(L, 3);

  int x = (int)luaL_checkinteger(L, 2);
  int y = (int)luaL_checkinteger(L, 3);

  plane_t *plane = get_plane(L, 1);

  lua_pushinteger(L, try_get(x, y, plane->width, plane->height, plane->buffer));

  lua_insert(L, 1);
  lua_settop(L, 1);

  return 1;
}

// modes:
//
// myplane:fill(3)
// myplane:fill(function(x, y, cur) return 2 + 1 end)
static int plane_fill(lua_State *L) {
  lua_settop(L, 2);

  plane_t *plane = get_plane(L, 1);
  size_t buffer_len = plane->width * plane->height;

  if (lua_isnumber(L, 2)) {
    int value = lua_tointeger(L, 2);
    for (int i = 0; i < buffer_len; ++i) {
      plane->buffer[i] = value;
    }
  } else if (lua_isfunction(L, 2)) {
    apply_func_to_buffer(L, 2, plane->width, plane->height, plane->buffer);
  }

  // return the plane
  lua_settop(L, 1);

  return 1;
}

// modes:
//
// myplane:set(x, y, 20)
static int plane_set(lua_State *L) {
  lua_settop(L, 4);

  int x = luaL_checkinteger(L, 2);
  int y = luaL_checkinteger(L, 3);

  plane_t *plane = get_plane(L, 1);
  plane->buffer[y * plane->width + x] = lua_tointeger(L, 4);

  // return the plane
  lua_settop(L, 1);

  return 1;
}

static plane_t *push_new_plane(int width, int height, size_t *len,
                               lua_State *L) {
  // ensure dimensions are sane
  if (width <= 0 || height <= 0) {
    LOG_SCRIPT_ERROR(L, "Invalid plane dimensions (%d, %d)", width, height);
    return NULL;
  }

  // wrap the userdata in a table so that we can assign __index metamethods
  lua_newtable(L);
  luaL_setmetatable(L, TBL_PLANE_META);

  lua_pushinteger(L, width);
  lua_setfield(L, -2, FIELD_PLANE_WIDTH);

  lua_pushinteger(L, height);
  lua_setfield(L, -2, FIELD_PLANE_HEIGHT);

  // let LuaJIT handle memory allocation and cleanup
  plane_t *plane = lua_newuserdata(L, sizeof(plane_t));
  plane->width = width;
  plane->height = height;

  // store the plane data in the wrapper table
  lua_setfield(L, -2, FIELD_PLANE_DATA);

  size_t buffer_len = (size_t)width * (size_t)height;
  plane->buffer = lua_newuserdata(L, buffer_len * sizeof(int));

  if (plane->buffer == NULL) {
    // if something went wrong, log a stack trace and bail
    LOG_SCRIPT_ERROR(L, "Failed to allocate memory for plane of size %zu",
                     buffer_len);
    return NULL;
  }

  // store a reference to the buffer in the wrapper table
  lua_setfield(L, -2, FIELD_PLANE_BUFFER);

  if (len != NULL) {
    *len = buffer_len;
  }

  return plane;
}

// plane:sub(x, y, w, h)
static int plane_sub(lua_State *L) {
  lua_settop(L, 5);

  int source_x = (int)luaL_checkinteger(L, 2);
  int source_y = (int)luaL_checkinteger(L, 3);
  int target_width = (int)luaL_checkinteger(L, 4);
  int target_height = (int)luaL_checkinteger(L, 5);

  plane_t *source = get_plane(L, 1);
  if (source == NULL) {
    LOG_SCRIPT_ERROR(L, "Invalid source plane");
    return 0;
  }

  plane_t *target = push_new_plane(target_width, target_height, NULL, L);

  if (target == NULL) {
    // error should already be logged by previous call
    return 0;
  }

  for (int x = 0; x < target_width; ++x) {
    for (int y = 0; y < target_height; ++y) {
      target->buffer[y * target_width + x] =
          try_get(source_x + x, source_y + y, source->width, source->height,
                  source->buffer);
    }
  }

  lua_settop(L, 1);

  return 1;
}

// plane:copy()
static int plane_copy(lua_State *L) {
  lua_settop(L, 2);

  plane_t *source = get_plane(L, 1);
  if (source == NULL) {
    LOG_SCRIPT_ERROR(L, "Invalid plane");
    return 0;
  }

  plane_t *copy = push_new_plane(source->width, source->height, NULL, L);
  if (copy == NULL) {
    LOG_SCRIPT_ERROR(L, "Failed to copy plane buffer");
    return 0;
  }

  memcpy(copy->buffer, source->buffer,
         sizeof(int) * source->width * source->height);

  lua_settop(L, 1);

  return 1;
}

static int plane_decode(lua_State *L) { return 0; }

static int plane_encode(lua_State *L) {
  lua_settop(L, 1);

  plane_t *plane = get_plane(L, 1);
  if (plane == NULL) {
    LOG_SCRIPT_ERROR(L, "Invalid plane");
    return 0;
  }

  const uint32_t *planebuf = (uint32_t *)plane->buffer;

  if (planebuf == NULL) {
    LOG_SCRIPT_ERROR(L, "Invalid plane buffer");
    return 0;
  }

  const size_t planebuf_length = plane->width * plane->height;

  // compress the plane's buffer with SIMD magic
  unsigned int maxbit = maxbits_length(planebuf, planebuf_length);
  log_warn("bit: %u", maxbit);
  uint8_t *compressed =
      malloc(simdpack_compressedbytes(planebuf_length, maxbit));
  if (compressed == NULL) {
    LOG_SCRIPT_ERROR(L, "Failed to allocate the compressed plane buffer",
                     maxbit);
    return 0;
  }

  __m128i *compressed_end =
      simdpack_length(planebuf, planebuf_length, (__m128i *)compressed, maxbit);
  size_t compressed_size =
      (compressed_end - (__m128i *)compressed) * sizeof(__m128i);

  log_debug(
      "Plane of size %zu was compressed to %zu bytes (%.1f%% of original)",
      planebuf_length * sizeof(int), compressed_size,
      ((float)compressed_size / (float)(planebuf_length * sizeof(int)) *
       100.0F));

  char *encoded = base64_enc_malloc(compressed, compressed_size);
  free(compressed);

  if (encoded == NULL) {
    LOG_SCRIPT_ERROR(L, "Failed to base64-encode the plane buffer");
    return 0;
  }

  lua_pushstring(L, encoded);

  free(encoded);

  return 1;
}

// plane:blit(x, y, src)
static int plane_blit(lua_State *L) {
  lua_settop(L, 4);

  int offset_x = luaL_checkinteger(L, 2);
  int offset_y = luaL_checkinteger(L, 3);

  plane_t *dest = get_plane(L, 1);
  if (dest == NULL) {
    LOG_SCRIPT_ERROR(L, "Invalid destination plane");
    return 0;
  }

  plane_t *src = get_plane(L, 4);
  if (src == NULL) {
    LOG_SCRIPT_ERROR(L, "Invalid source plane");
    return 0;
  }

  for (int x = 0; x < src->width; ++x) {
    if (x + offset_x < 0 || x + offset_x >= dest->width) {
      continue;
    }

    for (int y = 0; y < src->height; ++y) {
      if (y + offset_y < 0 || y + offset_y >= dest->height) {
        continue;
      }

      dest->buffer[(y + offset_y) * dest->width + (x + offset_x)] =
          src->buffer[y * src->width + x];
    }
  }

  lua_settop(L, 1);

  return 1;
}

// modes:
//
// plane.from(w, h, 4)
// plane.from(w, h, function(x, y) return x + y end)
static int plane_from(lua_State *L) {
  lua_settop(L, 3);

  // fetch plane dimensions from first two arguments
  int width = luaL_checkinteger(L, 1);
  int height = luaL_checkinteger(L, 2);

  size_t buffer_len;
  plane_t *plane = push_new_plane(width, height, &buffer_len, L);
  if (plane == NULL) {
    return 0;
  }

  if (lua_isnumber(L, 3)) {
    int value = (int)lua_tointeger(L, 3);
    for (int i = 0; i < buffer_len; ++i) {
      plane->buffer[i] = value;
    }
  } else if (lua_isfunction(L, 3)) {
    apply_func_to_buffer(L, 3, plane->width, plane->height, plane->buffer);
  }

  lua_insert(L, 1);

  lua_settop(L, 1);

  return 1;
}

static int plane_get_size(lua_State *L) {
  lua_settop(L, 1);

  plane_t *plane = get_plane(L, 1);
  if (plane == NULL) {
    LOG_SCRIPT_ERROR(L, "Invalid plane");
    return 0;
  }

  lua_pushinteger(L, plane->width);
  lua_pushinteger(L, plane->height);

  return 2;
}

void add_plane(lua_State *L) {
  // initialize library table
  luaL_Reg create_methods[] = {{FUNC_PLANE_FROM, plane_from}, {NULL, NULL}};
  luaL_newlib(L, create_methods);
  lua_setfield(L, 1, TBL_PLANE);

  // initialize metatable
  if (luaL_newmetatable(L, TBL_PLANE_META)) {
    luaL_Reg index_methods[] = {{FUNC_PLANE_AT, plane_at},
                                {FUNC_PLANE_SET, plane_set},
                                {FUNC_PLANE_FILL, plane_fill},
                                {FUNC_PLANE_FOREACH, plane_fill},
                                {FUNC_PLANE_SUB, plane_sub},
                                {FUNC_PLANE_COPY, plane_copy},
                                {FUNC_PLANE_ENCODE, plane_encode},
                                {FUNC_PLANE_DECODE, plane_decode},
                                {FUNC_PLANE_GETSIZE, plane_get_size},
                                {FUNC_PLANE_BLIT, plane_blit},
                                {NULL, NULL}};
    luaL_newlib(L, index_methods);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
  }
}
