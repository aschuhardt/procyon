#include "shader.h"

#include <stdlib.h>

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <log.h>

typedef procy_shader_program_t shader_program_t;

static bool compile_shader(const char* data, int shader_type, GLuint* index) {
  *index = glCreateShader(shader_type);
  const GLchar* vert_source[1] = {data};
  glShaderSource(*index, 1, vert_source, NULL);
  glCompileShader(*index);

  // check that the shader compiled correctly
  GLint compiled;
  glGetShaderiv(*index, GL_COMPILE_STATUS, &compiled);
  if (compiled != GL_TRUE) {
    GLsizei msg_len = 0;
    GLchar msg[1024];
    glGetShaderInfoLog(*index, sizeof(msg) / sizeof(GLchar), &msg_len, msg);
    if (shader_type == GL_VERTEX_SHADER) {
      log_error("Failed to compile vertex shader: %s", msg);
    } else if (shader_type == GL_FRAGMENT_SHADER) {
      log_error("Failed to compile fragment shader: %s", msg);
    }
    return false;
  }

  return true;
}

bool procy_link_shader_program(unsigned int vert, unsigned int frag,
                               unsigned int* index) {
  *index = glCreateProgram();
  glAttachShader(*index, vert);
  glAttachShader(*index, frag);
  glLinkProgram(*index);

  // check that no errors occured during linking
  GLint linked;
  glGetProgramiv(*index, GL_LINK_STATUS, &linked);
  if (linked != GL_TRUE) {
    GLsizei msg_len = 0;
    GLchar msg[1024];
    glGetProgramInfoLog(*index, sizeof(msg) / sizeof(GLchar), &msg_len, msg);
    log_error("Failed to link shader program: %s", msg);
    return false;
  }

  return true;
}

bool procy_compile_vert_shader(const char* data, unsigned int* index) {
  return compile_shader(data, GL_VERTEX_SHADER, index);
}

bool procy_compile_frag_shader(const char* data, unsigned int* index) {
  return compile_shader(data, GL_FRAGMENT_SHADER, index);
}

void procy_destroy_shader_program(shader_program_t* shader) {
  // detach shaders from program and delete program
  if (glIsProgram(shader->program) == GL_TRUE) {
    if (glIsShader(shader->fragment) == GL_TRUE) {
      glDetachShader(shader->program, shader->fragment);
    }
    if (glIsShader(shader->vertex) == GL_TRUE) {
      glDetachShader(shader->program, shader->vertex);
    }
    glDeleteProgram(shader->program);
  }

  // delete shaders
  if (glIsShader(shader->fragment) == GL_TRUE) {
    glDeleteShader(shader->fragment);
  }
  if (glIsShader(shader->vertex) == GL_TRUE) {
    glDeleteShader(shader->vertex);
  }

  // delete vertex buffers
  if (shader->vbo_count > 0 && shader->vbo != NULL) {
    glDeleteBuffers(shader->vbo_count, shader->vbo);
    free(shader->vbo);
  }

  // delete vertex array
  if (glIsVertexArray(shader->vao)) {
    glDeleteVertexArrays(1, &shader->vao);
  }

  shader->valid = false;
}

