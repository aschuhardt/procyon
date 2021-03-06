cmake_minimum_required(VERSION 3.17.0)

project(procyon-engine VERSION 0.1.0 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os")

add_subdirectory(lib)

set(SU_SOURCE
  src/drawing.c
  src/window.c
  src/state.c
  src/shader.c
  src/keys.c
  src/color.c
  src/shader/glyph.c
  src/shader/rect.c
  src/shader/line.c
  src/shader/error.c)
set(SU_INCLUDE
  "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SU_LIBRARY procyon)

get_directory_property(parentDir PARENT_DIRECTORY)
if(parentDir)
  set(SU_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include" PARENT_SCOPE)
  set(SU_LIBRARY procyon PARENT_SCOPE)
endif()

if(SU_SHARED)
  add_library(${SU_LIBRARY} SHARED ${SU_SOURCE})
  # target_link_options(${SU_LIBRARY} PUBLIC -fPIC)
else()
  add_library(${SU_LIBRARY} ${SU_SOURCE})
endif()

target_include_directories(${SU_LIBRARY}
  PUBLIC
  ${SU_INCLUDE}
  ${LOG_INCLUDE_DIR}
  ${STB_INCLUDE_DIR})
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
  cp437.png
  cp437_bold.png
  rect.vert
  rect.frag
  line.vert
  line.frag)

# ... and their corresponding header file names ...
list(APPEND EMBED_HEADERS
  glyph_vert.h
  glyph_frag.h
  tileset.h
  tileset_bold.h
  rect_vert.h
  rect_frag.h
  line_vert.h
  line_frag.h)

# ... and specify target names for each embedded object
list(APPEND EMBED_TARGETS
  embed_glyph_vert
  embed_glyph_frag
  embed_tileset
  embed_tileset_bold
  embed_rect_vert
  embed_rect_frag
  embed_line_vert
  embed_line_frag)

# create a directory for generated files to be placed into
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/gen/")

# Convert each embedded resource into a C header file
foreach(filename header target IN ZIP_LISTS EMBED_FILES EMBED_HEADERS EMBED_TARGETS)
  set(header "${CMAKE_CURRENT_SOURCE_DIR}/include/gen/${header}")
  set(filename "${CMAKE_CURRENT_SOURCE_DIR}/res/${filename}")
  add_custom_command(
    DEPENDS genhexer ${filename}
    OUTPUT ${header}
    COMMAND genhexer ${filename} ${header} ${target}
    USES_TERMINAL)
  set_source_files_properties(${header} PROPERTIES GENERATED 1)
  add_custom_target(${target} DEPENDS ${header})
  add_dependencies(${SU_LIBRARY} ${target})
endforeach()

# build an implementation of a Lua wrapper for lib
add_subdirectory(lua)

# build UI toolkit
# add_subdirectory(procyon-ui)

# add benchmark tools
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/bench")
