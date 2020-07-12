cmake_minimum_required(VERSION 3.17.2)

project(procyon-engine VERSION 0.1.0 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message("Building in debug mode")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -DDEBUG -ggdb -g3")
else()
  message("Building in release mode")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
endif()

add_subdirectory(lib)

set(SU_SOURCE
  src/main.c
  src/drawing.c
  src/window.c
  src/script.c
  src/config.c
  src/shader.c
  src/script/drawing.c
  src/script/window.c
  src/script/utility.c
  src/script/globals.c
  src/script/input.c
  src/script/environment.c)

add_executable(procyon ${SU_SOURCE})
target_include_directories(procyon 
  PRIVATE include ${LUA_INCLUDE_DIR} ${ARGPARSE_INCLUDE_DIR} ${STB_INCLUDE_DIR})
target_link_libraries(procyon 
  PRIVATE ${LUA_LIBRARIES} argparse log glad glfw liblua)
set_property(TARGET procyon PROPERTY C_STANDARD 11)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(procyon PRIVATE EXPORT_GLYPH_BITMAP=1)
endif()

add_custom_target(script_test
  COMMAND procyon --debug -e "${PROJECT_SOURCE_DIR}/sample/test.lua"
  DEPENDS procyon
  USES_TERMINAL)

# Compile a small program that will convert files into
# C header files
add_executable(genhexer util/genhexer.c)

# Here are lists containing the files that we want to embed
# into C header files...
#
# Note: these must be placed in the "res" directory at the
#       project root
list(APPEND EMBED_FILES
  glyph.vert
  glyph.frag
  LessPerfectDOSVGA.ttf)

# ... and their corresponding header file names ...
list(APPEND EMBED_HEADERS
  glyph_vert.h
  glyph_frag.h
  vga_ttf.h)

# ... and specify target names for each embedded object
list(APPEND EMBED_TARGETS
  embed_glyph_vert
  embed_glyph_frag
  embed_vga_ttf)

# Convert each embedded resource into a C header file
foreach(filename header target IN ZIP_LISTS EMBED_FILES EMBED_HEADERS EMBED_TARGETS)
  set(header "${CMAKE_CURRENT_SOURCE_DIR}/include/gen/${header}")
  set(filename "${CMAKE_CURRENT_SOURCE_DIR}/res/${filename}")
  add_custom_command(
    DEPENDS genhexer ${filename}
    OUTPUT ${header}
    COMMAND genhexer ${filename} ${header} ${target})
  set_source_files_properties(${header} PROPERTIES GENERATED 1)
  add_custom_target(${target} DEPENDS ${header})
  add_dependencies(procyon ${target})
endforeach()
