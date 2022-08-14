#include "shader.h"

#include <stdlib.h>

// clang-format off
#include "opengl.h"
#include <GLFW/glfw3.h>
// clang-format on

#include <log.h>

#include "shader/error.h"

typedef procy_shader_program_t shader_program_t;

static bool compile_shader(const char *data, int shader_type, GLuint *index) {
  *index = glCreateShader(shader_type);
  const GLchar *vert_source[1] = {data};
  GL_CHECK(glShaderSource(*index, 1, vert_source, NULL));
  GL_CHECK(glCompileShader(*index));

  // check that the shader compiled correctly
  GLint compiled;
  GL_CHECK(glGetShaderiv(*index, GL_COMPILE_STATUS, &compiled));
  if (compiled != GL_TRUE) {
    GLsizei msg_len = 0;
    GLchar msg[1024];
    GL_CHECK(glGetShaderInfoLog(*index, sizeof(msg) / sizeof(GLchar), &msg_len,
                                msg));
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
                               unsigned int *index) {
  *index = glCreateProgram();
  GL_CHECK(glAttachShader(*index, vert));
  GL_CHECK(glAttachShader(*index, frag));
  GL_CHECK(glLinkProgram(*index));

  // clear shader resources
  glDetachShader(*index, vert);
  glDeleteShader(vert);
  glDetachShader(*index, frag);
  glDeleteShader(frag);

  // check that no errors occured during linking
  GLint linked;
  GL_CHECK(glGetProgramiv(*index, GL_LINK_STATUS, &linked));
  if (linked != GL_TRUE) {
    GLsizei msg_len = 0;
    GLchar msg[1024];
    GL_CHECK(glGetProgramInfoLog(*index, sizeof(msg) / sizeof(GLchar), &msg_len,
                                 msg));
    log_error("Failed to link shader program: %s", msg);
    return false;
  }

  return true;
}

bool procy_compile_vert_shader(const char *data, unsigned int *index) {
  return compile_shader(data, GL_VERTEX_SHADER, index);
}

bool procy_compile_frag_shader(const char *data, unsigned int *index) {
  return compile_shader(data, GL_FRAGMENT_SHADER, index);
}

void procy_destroy_shader_program(shader_program_t *shader) {
  if (glIsProgram(shader->program)) {
    glDeleteProgram(shader->program);
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
}

bool procy_compile_and_link_shader(procy_shader_program_t *program,
                                   const char *vert, const char *frag) {
  return procy_compile_frag_shader(frag, &program->fragment) &&
         procy_compile_vert_shader(vert, &program->vertex) &&
         procy_link_shader_program(program->vertex, program->fragment,
                                   &program->program);
}
