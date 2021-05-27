#include <lauxlib.h>
#include <lua.h>

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

#include "script.h"
#include "script/environment.h"

#define TBL_NOISE "noise"

#define FUNC_NOISE_PERLIN "perlin"
#define FUNC_NOISE_RIDGE "ridge"
#define FUNC_NOISE_FBM "fbm"
#define FUNC_NOISE_TURBULENCE "turbulence"

#define DEFAULT_LACUNARITY 2.0F
#define DEFAULT_GAIN 0.5F
#define DEFAULT_OFFSET 1.0F
#define DEFAULT_OCTAVES 6

static int noise_perlin(lua_State *L) {
  float value = 0.0F;

  // last argument is an optional seed value (note that for the seed, only the
  // bottom 8 bits are used)
  if (lua_gettop(L) == 3) {
    value = stb_perlin_noise3(lua_tonumber(L, 1), lua_tonumber(L, 2),
                              lua_tonumber(L, 3), 0, 0, 0);
  } else if (lua_gettop(L) == 4) {
    value = stb_perlin_noise3_seed(lua_tonumber(L, 1), lua_tonumber(L, 2),
                                   lua_tonumber(L, 3), 0, 0, 0,
                                   lua_tointeger(L, 4));
  }

  lua_pushnumber(L, value);

  return 1;
}

static int noise_ridge(lua_State *L) {
  float lacunarity = DEFAULT_LACUNARITY;
  float gain = DEFAULT_GAIN;
  float offset = DEFAULT_OFFSET;
  int octaves = DEFAULT_OCTAVES;

  if (lua_gettop(L) == 7) {
    lacunarity = lua_tonumber(L, 4);
    gain = lua_tonumber(L, 5);
    offset = lua_tonumber(L, 6);
    octaves = lua_tointeger(L, 7);
  }

  float value = stb_perlin_ridge_noise3(lua_tonumber(L, 1), lua_tonumber(L, 2),
                                        lua_tonumber(L, 3), lacunarity, gain,
                                        offset, octaves);
  lua_pushnumber(L, value);

  return 1;
}

static int noise_fbm(lua_State *L) {
  float lacunarity = DEFAULT_LACUNARITY;
  float gain = DEFAULT_GAIN;
  int octaves = DEFAULT_OCTAVES;

  if (lua_gettop(L) == 6) {
    lacunarity = lua_tonumber(L, 4);
    gain = lua_tonumber(L, 5);
    octaves = lua_tointeger(L, 6);
  }

  float value =
      stb_perlin_fbm_noise3(lua_tonumber(L, 1), lua_tonumber(L, 2),
                            lua_tonumber(L, 3), lacunarity, gain, octaves);
  lua_pushnumber(L, value);

  return 1;
}

static int noise_turbulence(lua_State *L) {
  float lacunarity = DEFAULT_LACUNARITY;
  float gain = DEFAULT_GAIN;
  int octaves = DEFAULT_OCTAVES;

  if (lua_gettop(L) == 6) {
    lacunarity = lua_tonumber(L, 4);
    gain = lua_tonumber(L, 5);
    octaves = lua_tointeger(L, 6);
  }

  float value = stb_perlin_turbulence_noise3(
      lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lacunarity,
      gain, octaves);
  lua_pushnumber(L, value);

  return 1;
}

void add_noise(lua_State *L) {
  luaL_Reg methods[] = {{FUNC_NOISE_PERLIN, noise_perlin},
                        {FUNC_NOISE_RIDGE, noise_ridge},
                        {FUNC_NOISE_FBM, noise_fbm},
                        {FUNC_NOISE_TURBULENCE, noise_turbulence},
                        {NULL, NULL}};
  luaL_newlib(L, methods);
  lua_setglobal(L, TBL_NOISE);
}
