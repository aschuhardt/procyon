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

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb")
set(CMAKE_C_FLAGS_RELEASE 
  "${CMAKE_C_FLAGS_RELEASE} -ffunction-sections -flto -Os -fmerge-all-constants -fdata-sections")

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
  pkg_check_modules(LUAJIT luajit)
endif()

add_executable(procyon-lua ${SU_LUA_SOURCE})
target_link_libraries(procyon-lua PUBLIC ${SU_LIBRARY} argparse ${LUAJIT_LIBRARIES} m)

if(UNIX)
  target_link_libraries(procyon-lua PUBLIC dl)
endif()

target_include_directories(procyon-lua
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
  ${STB_INCLUDE_DIR} ${ARGPARSE_INCLUDE_DIR} ${LOG_INCLUDE_DIR} ${SU_INCLUDE} ${LUAJIT_INCLUDE_DIRS}) 
set_property(TARGET procyon-lua PROPERTY EXCLUDE_FROM_ALL TRUE)

add_custom_target(copy_spritesheet_for_script_test
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/sample/sprites.png
    ${CMAKE_CURRENT_BINARY_DIR}/sprites.png
  USES_TERMINAL)

add_custom_target(script_test
  COMMAND procyon-lua --debug -e "${CMAKE_CURRENT_SOURCE_DIR}/sample/main.lua"
  DEPENDS procyon-lua copy_spritesheet_for_script_test
  USES_TERMINAL)

add_custom_target(mouse_test
  COMMAND procyon-lua --debug -e "${CMAKE_CURRENT_SOURCE_DIR}/sample/mouse.lua"
  DEPENDS procyon-lua
  USES_TERMINAL)
