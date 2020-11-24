#include <lauxlib.h>
#include <log.h>
#include <lua.h>
#include <string.h>

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

#include "procyon.h"
#include "script.h"
#include "script/environment.h"

#define TBL_LOG "log"
#define TBL_NOISE "noise"
#define TBL_SCRIPT "script"

#define FUNC_LOG_INFO "info"
#define FUNC_LOG_ERROR "error"
#define FUNC_LOG_WARN "warn"
#define FUNC_LOG_DEBUG "debug"
#define FUNC_NOISE_PERLIN "perlin"
#define FUNC_NOISE_RIDGE "ridge"
#define FUNC_NOISE_FBM "fbm"
#define FUNC_NOISE_TURBULENCE "turbulence"
#define FUNC_SCRIPT_RUN "run"

#define DEFAULT_LACUNARITY 2.0F
#define DEFAULT_GAIN 0.5F
#define DEFAULT_OFFSET 1.0F
#define DEFAULT_OCTAVES 6

static int write_log_info(lua_State *L) {
  if (verify_arg_count(L, 1, __func__)) {
    log_info("%s", lua_tostring(L, 1));
  }

  return 0;
}

static int write_log_error(lua_State *L) {
  if (verify_arg_count(L, 1, __func__)) {
    log_error("%s", lua_tostring(L, 1));
  }

  return 0;
}

static int write_log_warn(lua_State *L) {
  if (verify_arg_count(L, 1, __func__)) {
    log_warn("%s", lua_tostring(L, 1));
  }

  return 0;
}

static int write_log_debug(lua_State *L) {
  if (verify_arg_count(L, 1, __func__)) {
    log_debug("%s", lua_tostring(L, 1));
  }

  return 0;
}

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

  lua_pop(L, lua_gettop(L));
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
  lua_pop(L, lua_gettop(L));
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
  lua_pop(L, lua_gettop(L));
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
  lua_pop(L, lua_gettop(L));
  lua_pushnumber(L, value);

  return 1;
}

static int run_script(lua_State *L) {
  luaL_dostring(L, lua_tostring(L, 1));

  return 0;
}

static void add_logging(lua_State *L) {
  lua_newtable(L);

  lua_pushcfunction(L, write_log_info);
  lua_setfield(L, -2, FUNC_LOG_INFO);

  lua_pushcfunction(L, write_log_error);
  lua_setfield(L, -2, FUNC_LOG_ERROR);

  lua_pushcfunction(L, write_log_warn);
  lua_setfield(L, -2, FUNC_LOG_WARN);

  lua_pushcfunction(L, write_log_debug);
  lua_setfield(L, -2, FUNC_LOG_DEBUG);

  lua_setglobal(L, TBL_LOG);
}

static void add_noise(lua_State *L) {
  lua_newtable(L);

  lua_pushcfunction(L, noise_perlin);
  lua_setfield(L, -2, FUNC_NOISE_PERLIN);

  lua_pushcfunction(L, noise_ridge);
  lua_setfield(L, -2, FUNC_NOISE_RIDGE);

  lua_pushcfunction(L, noise_fbm);
  lua_setfield(L, -2, FUNC_NOISE_FBM);

  lua_pushcfunction(L, noise_turbulence);
  lua_setfield(L, -2, FUNC_NOISE_TURBULENCE);

  lua_setglobal(L, TBL_NOISE);
}

static void add_script(lua_State *L) {
  lua_newtable(L);

  lua_pushcfunction(L, run_script);
  lua_setfield(L, -2, FUNC_SCRIPT_RUN);

  lua_setglobal(L, TBL_SCRIPT);
}

void add_utilities(lua_State *L) {
  add_logging(L);
  add_noise(L);
  add_script(L);
}
