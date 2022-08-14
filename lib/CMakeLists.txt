if (NOT EMSCRIPTEN)
  add_subdirectory(glad)
  add_subdirectory(glfw)
endif()

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

if (NOT EMSCRIPTEN)
  set(SIMDCOMP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/simdcomp")
  set(SIMDCOMP_INCLUDE_DIR "${SIMDCOMP_DIR}/include")
  set(SIMDCOMP_INCLUDE_DIR "${SIMDCOMP_DIR}/include" PARENT_SCOPE)
  add_library(simdcomp STATIC
    ${SIMDCOMP_DIR}/src/simdbitpacking.c
    ${SIMDCOMP_DIR}/src/simdintegratedbitpacking.c
    ${SIMDCOMP_DIR}/src/simdpackedselect.c
    ${SIMDCOMP_DIR}/src/simdcomputil.c)
  target_include_directories(simdcomp PUBLIC ${SIMDCOMP_INCLUDE_DIR})
  target_compile_options(simdcomp PRIVATE -std=c89)
endif()

set(BASE64_DIR "${CMAKE_CURRENT_SOURCE_DIR}/base64")
set(BASE64_INCLUDE_DIR ${BASE64_DIR} PARENT_SCOPE)
add_library(base64 STATIC "${BASE64_DIR}/base64.c")

set(WFC_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/wfc" PARENT_SCOPE)
