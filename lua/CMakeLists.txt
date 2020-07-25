# Lua application on top of procyon game engine
set(SU_LUA_SOURCE
  src/main.c
  src/script.c
  src/config.c
  src/script/drawing.c
  src/script/window.c
  src/script/utility.c
  src/script/globals.c
  src/script/input.c
  src/script/environment.c)

add_executable(procyon-lua ${SU_LUA_SOURCE})
target_include_directories(procyon-lua
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include"
          ${SU_INCLUDE} ${LUA_INCLUDE_DIR} ${ARGPARSE_INCLUDE_DIR}) 
target_link_libraries(procyon-lua
  PRIVATE argparse liblua procyon)

add_custom_target(script_test
  COMMAND procyon-lua --debug -s 2.0 -e "${CMAKE_CURRENT_SOURCE_DIR}/sample/test.lua"
  DEPENDS procyon-lua
  USES_TERMINAL)
