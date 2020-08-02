cmake_minimum_required(VERSION 3.17.2)

project(procyon-engine VERSION 0.1.0 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(lib)

set(SU_SOURCE
  src/drawing.c
  src/window.c
  src/shader.c
  src/keys.c)
set(SU_INCLUDE
  "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SU_LIBRARY procyon)

add_library(${SU_LIBRARY} ${SU_SOURCE})
target_include_directories(${SU_LIBRARY} 
  PRIVATE ${STB_INCLUDE_DIR})
target_include_directories(${SU_LIBRARY} PUBLIC ${SU_INCLUDE} ${LOG_INCLUDE_DIR})
target_link_libraries(${SU_LIBRARY} PRIVATE glad glfw)
target_link_libraries(${SU_LIBRARY} PUBLIC log)

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
  cp437.png)

# ... and their corresponding header file names ...
list(APPEND EMBED_HEADERS
  glyph_vert.h
  glyph_frag.h
  tileset.h)

# ... and specify target names for each embedded object
list(APPEND EMBED_TARGETS
  embed_glyph_vert
  embed_glyph_frag
  embed_tileset)

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
  add_dependencies(${SU_LIBRARY} ${target})
endforeach()

# build an implementation of a Lua wrapper for lib${SU_LIBRARY}
add_subdirectory(lua)

