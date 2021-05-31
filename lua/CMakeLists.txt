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
  src/script/noise.c
  src/script/plane.c
  src/script/environment.c)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -ffunction-sections -flto -O3 -fmerge-all-constants -fno-stack-protector -fdata-sections")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb")

add_executable(procyon-lua ${SU_LUA_SOURCE})
target_link_libraries(procyon-lua PUBLIC procyon argparse lua_static)
target_include_directories(procyon-lua
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
  ${STB_INCLUDE_DIR} ${ARGPARSE_INCLUDE_DIR} ${LOG_INCLUDE_DIR} ${SU_INCLUDE}) 
set_property(TARGET procyon-lua PROPERTY EXCLUDE_FROM_ALL TRUE)

add_custom_target(copy_spritesheet_for_script_test
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/sample/sprites.png
    ${CMAKE_CURRENT_BINARY_DIR}/sprites.png
  USES_TERMINAL)

add_custom_target(script_test
  COMMAND procyon-lua --debug -e "${CMAKE_CURRENT_SOURCE_DIR}/sample/test.lua"
  DEPENDS procyon-lua copy_spritesheet_for_script_test
  USES_TERMINAL)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
  add_custom_command(
    TARGET procyon-lua
    POST_BUILD
    COMMAND strip -S --strip-unneeded --remove-section=.note.gnu.gold-version --remove-section=.comment --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag $<TARGET_FILE:procyon-lua>)
  add_custom_command(
    TARGET procyon-lua
    POST_BUILD
    USES_TERMINAL
    COMMAND upx --lzma $<TARGET_FILE:procyon-lua>)
endif()
