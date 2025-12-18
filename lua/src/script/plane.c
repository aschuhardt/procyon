#include <base64.h>
#include <lauxlib.h>
#include <log.h>
#include <lua.h>
#include <stdlib.h>

#ifndef __EMSCRIPTEN__
#include <simdbitpacking.h>
#include <simdcomp.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>
#define WFC_IMPLEMENTATION
#include <wfc.h>

#include "script/environment.h"

#define TBL_PLANE "plane"
#define TBL_PLANE_META "procyon_plane_meta"

#define FUNC_PLANE_FROM "from"
#define FUNC_PLANE_FROM_WFC "from_wfc"
#define FUNC_PLANE_IMPORT "import"
#define FUNC_PLANE_EXPORT "export"
#define FUNC_PLANE_FILL "fill"
#define FUNC_PLANE_FOREACH "foreach"
#define FUNC_PLANE_AT "at"
#define FUNC_PLANE_SET "set"
#define FUNC_PLANE_SUB "sub"
#define FUNC_PLANE_COPY "copy"
#define FUNC_PLANE_BLIT "blit"
#define FUNC_PLANE_ENCODE "encode"
#define FUNC_PLANE_DECODE "decode"
#define FUNC_PLANE_FIND_FIRST "find_first"
#define FUNC_PLANE_FIND_ALL "find_all"
#define FUNC_PLANE_GETSIZE "get_size"
#define FIELD_PLANE_BUFFER "_buffer"
#define FIELD_PLANE_DATA "_data"
#define FIELD_PLANE_WIDTH "width"
#define FIELD_PLANE_HEIGHT "height"

#define WFC_DEFAULT_TILE_SIZE 3

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

static size_t store_at_offset(uint8_t *buffer, size_t offset, int value) {
  // store the provided integer in a buffer at offset, return the next available
  // offset
  buffer[offset++] = (value >> 24) & 0xFF;
  buffer[offset++] = (value >> 16) & 0xFF;
  buffer[offset++] = (value >> 8) & 0xFF;
  buffer[offset++] = value & 0xFF;
  return offset;
}

static int read_from_offset(uint8_t *buffer, size_t *offset) {
  int value = ((int)buffer[(*offset)++] & 0xFF) << 24;
  value |= ((int)buffer[(*offset)++] & 0xFF) << 16;
  value |= ((int)buffer[(*offset)++] & 0xFF) << 8;
  value |= ((int)buffer[(*offset)++] & 0xFF);

  return value;
}

// plane.decode(<base64>)
static int plane_decode(lua_State *L) {
  lua_settop(L, 1);

  const char *encoded = luaL_checkstring(L, 1);
  if (encoded == NULL) {
    LOG_SCRIPT_ERROR(L, "Invalid encoded plane string");
    return 0;
  }

  int buffer_size;
  uint8_t *buffer = base64_dec_malloc((char *)encoded, &buffer_size);
  if (buffer == NULL) {
    LOG_SCRIPT_ERROR(L, "Failed to decode base64 in plane string");
    return 0;
  }

  // pull metadata from the end of the decoded buffer
  size_t meta_offset = buffer_size - 3 * sizeof(int);
  int width = read_from_offset(buffer, &meta_offset);
  int height = read_from_offset(buffer, &meta_offset);
  uint32_t maxbit = read_from_offset(buffer, &meta_offset);

  log_debug("Decoded plane width: %d, height: %d, maxbit: %d", width, height,
            maxbit);

  // create a new plane and...
  size_t uncompressed_length = width * height;
  plane_t *plane = push_new_plane(width, height, NULL, L);
  if (plane == NULL) {
    return 0;
  }

  // ...decompress the buffer directly into it
  if (simdunpack_length((__m128i *)buffer, uncompressed_length,
                        (uint32_t *)plane->buffer, maxbit) == NULL) {
    LOG_SCRIPT_ERROR(L, "Failed to decompress plane data");
    memset(plane->buffer, 0, uncompressed_length * sizeof(int));
  }

  free(buffer);

  return 1;
}

static int plane_encode(lua_State *L) {
#ifdef __EMSCRIPTEN__
  LOG_SCRIPT_ERROR("Plane encoding isn't implemented for WASM yet.");
  return 0;
#else
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

  // resize the compressed buffer to hold only its data plus 3 ints (w, h,
  // maxbit)
  size_t buffer_size = compressed_size + 3 * sizeof(int);
  compressed = realloc(compressed, buffer_size);

  // store metadata after the compressed data
  size_t meta_offset =
      store_at_offset(compressed, compressed_size, plane->width);
  meta_offset = store_at_offset(compressed, meta_offset, plane->height);
  meta_offset = store_at_offset(compressed, meta_offset, maxbit);

  char *encoded = base64_enc_malloc(compressed, buffer_size);
  free(compressed);

  if (encoded == NULL) {
    LOG_SCRIPT_ERROR(L, "Failed to base64-encode the plane buffer");
    return 0;
  }

  lua_pushstring(L, encoded);

  free(encoded);

  return 1;
#endif
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
// plane.from({0, 2, 1}, {1, 3, 1}, {3, 1, 0})
static int plane_from(lua_State *L) {
  lua_settop(L, 3);

  if (lua_istable(L, 1)) {
    // using the list-of-rows signature
    int rows = lua_gettop(L);
    int columns = 0;
    for (int i = 1; i <= rows; ++i) {
      int len = lua_objlen(L, i);
      if (len > columns) {
        columns = len;
      }
    }

    if (rows == 0 || columns == 0) {
      LOG_SCRIPT_ERROR(L, "Invalid plane size; must be at least 1x1");
      return 0;
    }

    size_t buffer_len;
    plane_t *plane = push_new_plane(columns, rows, &buffer_len, L);
    if (plane == NULL) {
      return 0;
    }

    int width = 0;
    for (int y = 0; y < rows; ++y) {
      int row_width = lua_objlen(L, y + 1);
      for (int x = 0; x < row_width; ++x) {
        lua_rawgeti(L, y + 1, x + 1);
        plane->buffer[y * row_width + x] = lua_tonumber(L, -1);
        lua_pop(L, 1);
      }
    }
  } else if (lua_isnumber(L, 1) && lua_isnumber(L, 2)) {
    // using one of the (w, h, ...) signatures

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
  } else {
    LOG_SCRIPT_ERROR(L,
                     "Invalid call signature; expected (w, h, ...) or "
                     "or (w, h, function) or ({<row1>}, {<row2>}, ...)");
  }

  lua_insert(L, 1);
  lua_settop(L, 1);

  return 1;
}

// result = plane:find_first(function(x, y, value) return value % 2 == 0 end)
// {{ x, y, value }, { x, y, value }, ...}
static int plane_find_first(lua_State *L) {
  lua_settop(L, 2);

  plane_t *plane = get_plane(L, 1);
  for (size_t i = 0; i < (size_t)(plane->width * plane->height); ++i) {
    lua_pushvalue(L, 2);
    lua_pushinteger(L, (int)(i % plane->width));
    lua_pushinteger(L, (int)(i / plane->width));
    lua_pushinteger(L, (int)(plane->buffer[i]));

    if (lua_pcall(L, 3, 1, 0) != LUA_OK) {
      LOG_SCRIPT_ERROR(L, "Failed to find first element matching function");
      return 0;
    }

    if (lua_toboolean(L, -1)) {
      lua_newtable(L);
      lua_pushinteger(L, (int)(i % plane->width));
      lua_setfield(L, -2, "x");
      lua_pushinteger(L, (int)(i / plane->width));
      lua_setfield(L, -2, "y");
      lua_pushinteger(L, (int)(plane->buffer[i]));
      lua_setfield(L, -2, "value");

      return 1;
    }

    lua_pop(L, 1);
  }

  return 0;
}

// result = plane:find_all(function(x, y, value) return value % 2 == 0 end)
// { x, y, value }
static int plane_find_all(lua_State *L) {
  lua_settop(L, 2);

  plane_t *plane = get_plane(L, 1);

  // results will be appended onto this table
  lua_newtable(L);

  size_t count = 0;
  for (size_t i = 0; i < (size_t)(plane->width * plane->height); ++i) {
    lua_pushvalue(L, 2);
    lua_pushinteger(L, (int)(i % plane->width));
    lua_pushinteger(L, (int)(i / plane->width));
    lua_pushinteger(L, (int)(plane->buffer[i]));

    if (lua_pcall(L, 3, 1, 0) != LUA_OK) {
      LOG_SCRIPT_ERROR(L, "Failed to find first element matching function");
      return 0;
    }

    if (lua_toboolean(L, -1)) {
      lua_newtable(L);
      lua_pushinteger(L, (int)(i % plane->width));
      lua_setfield(L, -2, "x");
      lua_pushinteger(L, (int)(i / plane->width));
      lua_setfield(L, -2, "y");
      lua_pushinteger(L, (int)(plane->buffer[i]));
      lua_setfield(L, -2, "value");

      // the result table will be a list, so keys are 1-based indexes
      lua_rawseti(L, -3, ++count);
    }

    lua_pop(L, 1);
  }

  return 1;
}
// plane:export('output.png')
static int plane_export_image(lua_State *L) {
  lua_settop(L, 2);

  plane_t *plane = get_plane(L, 1);
  if (plane == NULL) {
    LOG_SCRIPT_ERROR(L, "Invalid plane");
    return 0;
  }

  size_t pixel_count = plane->width * plane->height;
  uint32_t *pixels = malloc(pixel_count * sizeof(uint32_t));
  memcpy(pixels, plane->buffer, pixel_count * sizeof(uint32_t));
  for (size_t i = 0; i < pixel_count; ++i) {
    pixels[i] |= 0xFF000000;  // set alpha to 255
  }

  const char *path = luaL_checkstring(L, 2);

  if (stbi_write_png(path, plane->width, plane->height, 4, pixels,
                     plane->width * sizeof(uint32_t)) == 0) {
    LOG_SCRIPT_ERROR(L, "Failed to export the plane");
  }

  log_debug("Exported plane to %s", path);

  free(pixels);

  return 0;
}

static plane_t *push_plane_from_image(lua_State *L, const char *path) {
  int width, height, comp;
  uint8_t *pixels = stbi_load(path, &width, &height, &comp, 4);
  if (pixels == NULL) {
    LOG_SCRIPT_ERROR(L, "Failed to import image");
    return 0;
  }

  size_t buffer_len;
  plane_t *plane = push_new_plane(width, height, &buffer_len, L);

  if (plane == NULL) {
    LOG_SCRIPT_ERROR(L, "Failed to create plane for image import");
  } else {
    memcpy((uint32_t *)plane->buffer, (uint32_t *)pixels,
           buffer_len * sizeof(uint32_t));
    for (size_t i = 0; i < buffer_len; ++i) {
      plane->buffer[i] &= 0x00FFFFFF;  // zero-out the 'alpha'
    }

    log_debug("Imported plane from %s", path);
  }

  stbi_image_free(pixels);

  return plane;
}

// p = plane.import('myplane.png')
static int plane_import_image(lua_State *L) {
  lua_settop(L, 1);

  const char *path = luaL_checkstring(L, 1);
  return push_plane_from_image(L, path) != NULL ? 1 : 0;
}

// plane.from_wfc(128, 128, "input.png", [flipx], [flipy], [nrot], [tilew],
// [tileh]) plane.from_wfc(128, 128, tile_plane)
static int plane_from_wfc(lua_State *L) {
  lua_settop(L, 8);

  int width = luaL_checkinteger(L, 1);
  int height = luaL_checkinteger(L, 2);
  bool flip_on_y = lua_toboolean(L, 5);
  bool flip_on_x = lua_toboolean(L, 4);
  int rotations = luaL_optint(L, 6, 0);
  int tile_width = luaL_optint(L, 7, WFC_DEFAULT_TILE_SIZE);
  int tile_height = luaL_optint(L, 8, WFC_DEFAULT_TILE_SIZE);

  // two ways to obtain the tile; either import it from a file path if arg
  // is a string, or use it directly if the arg is a plane
  plane_t *tile;
  if (lua_isstring(L, 3)) {
    tile = push_plane_from_image(L, lua_tostring(L, 3));
    if (tile == NULL) {
      LOG_SCRIPT_ERROR(L, "Failed to import WFC tile");
      return 0;
    }
  } else if (lua_istable(L, 3)) {
    tile = get_plane(L, 3);
    if (tile == NULL) {
      LOG_SCRIPT_ERROR(L, "Invalid tile plane");
      return 0;
    }
  } else {
    LOG_SCRIPT_ERROR(L, "Invalid WFC source, must be a plane or an image path");
    return 0;
  }

  // run the WFC iterations
  struct wfc_image tile_image = {(unsigned char *)tile->buffer, 4, tile->width,
                                 tile->height};
  struct wfc *wfc =
      wfc_overlapping(width, height, &tile_image, tile_width, tile_height, 1,
                      flip_on_x, flip_on_y, rotations);
  wfc_init(wfc);
  if (!wfc_run(wfc, -1)) {
    // don't log failure as it's an expected result
    lua_pushnil(L);
  } else {
    struct wfc_image *result = wfc_output_image(wfc);
    if (result == NULL) {
      LOG_SCRIPT_ERROR(L, "WFC resulted in no data");
    } else {
      // copy the resulting image into a new plane buffer
      size_t buffer_len;
      plane_t *plane =
          push_new_plane(result->width, result->height, &buffer_len, L);
      lua_insert(L, 1);
      lua_settop(L, 1);
      memcpy((uint8_t *)plane->buffer, (uint8_t *)result->data,
             buffer_len * sizeof(int));
      wfc_img_destroy(result);
    }
  }

  wfc_destroy(wfc);

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
  luaL_Reg create_methods[] = {{FUNC_PLANE_FROM, plane_from},
                               {FUNC_PLANE_FROM_WFC, plane_from_wfc},
                               {FUNC_PLANE_IMPORT, plane_import_image},
                               {FUNC_PLANE_DECODE, plane_decode},
                               {NULL, NULL}};
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
                                {FUNC_PLANE_EXPORT, plane_export_image},
                                {FUNC_PLANE_GETSIZE, plane_get_size},
                                {FUNC_PLANE_BLIT, plane_blit},
                                {FUNC_PLANE_FIND_FIRST, plane_find_first},
                                {FUNC_PLANE_FIND_ALL, plane_find_all},
                                {NULL, NULL}};
    luaL_newlib(L, index_methods);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
  }
}
