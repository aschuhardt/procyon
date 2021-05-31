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
  lua_settop(L, 4);

  float value = 0.0F;

  float x = luaL_checknumber(L, 1);
  float y = luaL_checknumber(L, 2);
  float z = luaL_checknumber(L, 3);

  if (lua_isnoneornil(L, 4)) {
    lua_pushnumber(L, stb_perlin_noise3(x, y, z, 0, 0, 0));
  } else {
    int seed = luaL_checkinteger(L, 4);
    lua_pushnumber(L, stb_perlin_noise3_seed(x, y, z, 0, 0, 0, seed));
  }

  return 1;
}

static int noise_ridge(lua_State *L) {
  lua_settop(L, 7);

  float x = luaL_checknumber(L, 1);
  float y = luaL_checknumber(L, 2);
  float z = luaL_checknumber(L, 3);
  float lacunarity = luaL_optnumber(L, 4, DEFAULT_LACUNARITY);
  float gain = luaL_optnumber(L, 5, DEFAULT_GAIN);
  float offset = luaL_optnumber(L, 6, DEFAULT_OFFSET);
  int octaves = luaL_optinteger(L, 7, DEFAULT_OCTAVES);

  lua_pushnumber(
      L, stb_perlin_ridge_noise3(x, y, z, lacunarity, gain, offset, octaves));

  return 1;
}

static int noise_fbm(lua_State *L) {
  lua_settop(L, 6);

  float x = luaL_checknumber(L, 1);
  float y = luaL_checknumber(L, 2);
  float z = luaL_checknumber(L, 3);
  float lacunarity = luaL_optnumber(L, 4, DEFAULT_LACUNARITY);
  float gain = luaL_optnumber(L, 5, DEFAULT_GAIN);
  int octaves = luaL_optinteger(L, 6, DEFAULT_OCTAVES);

  lua_pushnumber(L, stb_perlin_fbm_noise3(x, y, z, lacunarity, gain, octaves));

  return 1;
}

static int noise_turbulence(lua_State *L) {
  lua_settop(L, 6);

  float x = luaL_checknumber(L, 1);
  float y = luaL_checknumber(L, 2);
  float z = luaL_checknumber(L, 3);
  float lacunarity = luaL_optnumber(L, 4, DEFAULT_LACUNARITY);
  float gain = luaL_optnumber(L, 5, DEFAULT_GAIN);
  int octaves = luaL_optinteger(L, 6, DEFAULT_OCTAVES);

  lua_pushnumber(
      L, stb_perlin_turbulence_noise3(x, y, z, lacunarity, gain, octaves));

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
