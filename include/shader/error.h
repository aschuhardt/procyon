#ifndef SHADER_ERROR_H
#define SHADER_ERROR_H

#ifndef NDEBUG

const char* gl_error_string(int err);

void gl_print_errors(int line, const char* file);

#define GL_CHECK(expr) \
  expr;                \
  gl_print_errors(__LINE__, __FILE__);

#else
#define GL_CHECK(expr) (expr)
#endif

#endif
