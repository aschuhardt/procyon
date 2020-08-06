add_subdirectory(glad)
add_subdirectory(glfw)

add_library(tinydir INTERFACE)
target_include_directories(tinydir 
  INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/tinydir")

add_subdirectory(argparse)
set(ARGPARSE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/argparse" PARENT_SCOPE)
set(ARGPARSE_SHARED OFF)
set(ARGPARSE_STATIC ON)

set(LOG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/log/src")
set(LOG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/log/src" PARENT_SCOPE)
add_library(log "${LOG_INCLUDE_DIR}/log.c")
target_compile_definitions(log PRIVATE LOG_USE_COLOR)
target_include_directories(log PUBLIC ${LOG_INCLUDE_DIR})

set(STB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/stb" PARENT_SCOPE)
