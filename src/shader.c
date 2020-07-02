#include "shader.h"

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <log.h>

#include "gen/glyph_frag.h"
#include "gen/glyph_vert.h"
#include "window.h"

typedef struct glyph_vertex_t {
  float x, y;
} glyph_vertex_t;

static const size_t glyph_vbo_index_pos = 0;
static const size_t glyph_vbo_index_indicies = 1;
static const size_t glyph_attr_index_pos = 0;

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

static bool link_shader_program(GLuint vert, GLuint frag, GLuint* index) {
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

static bool compile_vert_shader(const char* data, GLuint* index) {
  return compile_shader(data, GL_VERTEX_SHADER, index);
}

static bool compile_frag_shader(const char* data, GLuint* index) {
  return compile_shader(data, GL_FRAGMENT_SHADER, index);
}

shader_program_t create_glyph_shader() {
  shader_program_t prog;
  prog.valid = compile_vert_shader((char*)embed_glyph_vert, &prog.vertex) &&
               compile_frag_shader((char*)embed_glyph_frag, &prog.fragment);

  // if shaders failed to compile, don't bother with the rest
  if (!prog.valid) {
    return prog;
  }

  // create vertex array
  glGenVertexArrays(1, &prog.vao);
  glBindVertexArray(prog.vao);

  // create vertex buffers
  prog.vbo_count = 2;

  glGenBuffers(prog.vbo_count, prog.vbo);

  prog.valid &= link_shader_program(prog.vertex, prog.fragment, &prog.program);

  return prog;
}

void draw_glyphs(shader_program_t* program, window_t* window, glyph_t* data,
                 size_t count) {
  glyph_vertex_t verts[count * 4];
  GLushort indices[count * 6];

  tile_bounds_t bounds = window->tile_bounds;
  window_bounds_t win_bounds = window->bounds;
  float tile_w = (float)(bounds.width + win_bounds.width % bounds.width);
  float tile_h = (float)(bounds.height + win_bounds.height % bounds.height);

  for (int i = 0; i < count; ++i) {
    float x = (float)data[i].x * tile_w;
    float y = (float)data[i].y * tile_h;

    size_t vert_ix = i * 4;

    // top left
    verts[vert_ix].x = x;
    verts[vert_ix].y = y;

    // top right
    verts[vert_ix + 1].x = x + tile_w;
    verts[vert_ix + 1].y = y;

    // bottom left
    verts[vert_ix + 2].x = x;
    verts[vert_ix + 2].y = y + tile_h;

    // bottom right
    verts[vert_ix + 3].x = x + tile_w;
    verts[vert_ix + 3].y = y + tile_h;

    size_t index_ix = i * 6;

    // first triangle
    indices[index_ix] = vert_ix;
    indices[index_ix + 1] = vert_ix + 1;
    indices[index_ix + 2] = vert_ix + 2;

    // second triangle
    indices[index_ix + 3] = vert_ix + 1;
    indices[index_ix + 4] = vert_ix + 3;
    indices[index_ix + 5] = vert_ix + 2;
  }

  glUseProgram(program->program);

  // set window size
  glUniform2f(glGetUniformLocation(program->program, "u_WindowSize"),
              (float)window->bounds.width, (float)window->bounds.height);

  // set tile size
  glUniform2f(glGetUniformLocation(program->program, "u_TileSize"),
              (float)window->tile_bounds.width,
              (float)window->tile_bounds.height);

  // copy vertex data to video memory
  glBindBuffer(GL_ARRAY_BUFFER, program->vbo[glyph_vbo_index_pos]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

  // associate the vertex data with the correct shader attribute
  glEnableVertexAttribArray(glyph_vbo_index_pos);
  glVertexAttribPointer(glyph_attr_index_pos, 2, GL_FLOAT, GL_FALSE, 0, 0);

  // copy indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, program->vbo[glyph_vbo_index_indicies]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]),
                 GL_UNSIGNED_SHORT, 0);

  glDisableVertexAttribArray(glyph_vbo_index_pos);
  glUseProgram(0);
}

void destroy_shader_program(shader_program_t* prog) {
  // detach shaders from program and delete program
  if (glIsProgram(prog->program) == GL_TRUE) {
    if (glIsShader(prog->fragment) == GL_TRUE) {
      glDetachShader(prog->program, prog->fragment);
    }
    if (glIsShader(prog->vertex) == GL_TRUE) {
      glDetachShader(prog->program, prog->vertex);
    }
    glDeleteProgram(prog->program);
  }

  // delete shaders
  if (glIsShader(prog->fragment) == GL_TRUE) {
    glDeleteShader(prog->fragment);
  }
  if (glIsShader(prog->vertex) == GL_TRUE) {
    glDeleteShader(prog->vertex);
  }

  // delete vertex buffers
  if (prog->vbo_count > 0 && prog->vbo != NULL) {
    glDeleteBuffers(prog->vbo_count, prog->vbo);
  }

  // delete vertex array
  if (glIsVertexArray(prog->vao)) {
    glDeleteVertexArrays(1, &prog->vao);
  }

  prog->valid = false;
}
