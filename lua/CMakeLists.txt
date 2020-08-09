find_package(Lua 5.4.0 EXACT)

if(LUA_FOUND)
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
  target_link_libraries(procyon-lua PUBLIC argparse ${LUA_LIBRARIES} ${SU_LIBRARY})
  target_include_directories(procyon-lua
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
    ${LUA_INCLUDE_DIR} ${ARGPARSE_INCLUDE_DIR} ${LOG_INCLUDE_DIR} ${SU_INCLUDE}) 
  
  add_custom_target(script_test
    COMMAND procyon-lua --debug -e "${CMAKE_CURRENT_SOURCE_DIR}/sample/test.lua"
    DEPENDS procyon-lua
    USES_TERMINAL)
endif()
