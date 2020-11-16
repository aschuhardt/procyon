add_executable(bench_framerate framerate.c)
target_include_directories(bench_framerate PRIVATE ${SU_INCLUDE})
target_link_libraries(bench_framerate PRIVATE ${SU_LIBRARY})
set_property(TARGET bench_framerate PROPERTY EXCLUDE_FROM_ALL TRUE)

add_custom_target(benchmarks
  COMMAND bench_framerate
  DEPENDS bench_framerate
  USES_TERMINAL)
