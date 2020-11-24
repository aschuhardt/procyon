#include "shader/error.h"

#include <glad/glad.h>
#include <log.h>

#ifndef NDEBUG

const char *gl_error_string(int err) {
  switch (err) {
  case GL_INVALID_ENUM:
    return "invalid enum";
  case GL_INVALID_VALUE:
    return "invalid value";
  case GL_INVALID_OPERATION:
    return "invalid operation";
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    return "invalid framebuffer operation";
  case GL_OUT_OF_MEMORY:
    return "out of memory";
  default:
    return "unknown OpenGL error";
  }
}

void gl_print_errors(int line, const char *expr, const char *file) {
  GLenum err = glGetError();
  while (err != GL_NO_ERROR) {
    log_error("OpenGL error calling %s (%s: %d): %s", expr, file, line,
              gl_error_string((int)err));
    err = glGetError();
  }
}

#endif
