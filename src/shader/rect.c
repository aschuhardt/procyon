#include "shader/rect.h"

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "drawing.h"
#include "window.h"
#include "gen/rect_frag.h"
#include "gen/rect_vert.h"
#include "shader/error.h"

typedef procy_rect_shader_program_t rect_shader_program_t;
typedef procy_window_t window_t;
typedef procy_color_t color_t;
typedef procy_shader_program_t shader_program_t;
typedef procy_draw_op_t draw_op_t;

#pragma pack(0)
typedef struct rect_vertex_t {
  float x, y;
  color_t color;
} rect_vertex_t;
#pragma pack(1)

static const size_t VBO_RECT_POSITION = 0;
static const size_t VBO_RECT_INDICES = 1;
static const size_t ATTR_RECT_POSITION = 0;
static const size_t ATTR_RECT_COLOR = 1;

static const size_t INDICES_PER_RECT = 6;
static const size_t VERTICES_PER_RECT = 4;

static size_t count_rects_in_ops_buffer(draw_op_t* ops, size_t n) {
  size_t count = 0;
  for (int i = 0; i < n; i++) {
    if (ops[i].type == DRAW_OP_RECT) {
      ++count;
    }
  }

  return count;
}

static void update_buffer_sizes(rect_shader_program_t* shader,
                                size_t rect_count) {
  if (rect_count != shader->rect_count) {
    shader->index_buffer = realloc(
        shader->index_buffer, sizeof(GLushort) * INDICES_PER_RECT * rect_count);
    shader->vertex_buffer =
        realloc(shader->vertex_buffer,
                sizeof(rect_vertex_t) * VERTICES_PER_RECT * rect_count);
    shader->rect_count = rect_count;
  }
}

/* --------------------------- */
/* Public interface definition */
/* --------------------------- */

rect_shader_program_t* procy_create_rect_shader() {
  rect_shader_program_t* rect_shader = malloc(sizeof(rect_shader_program_t));

  rect_shader->index_buffer = NULL;
  rect_shader->vertex_buffer = NULL;
  rect_shader->rect_count = 0;

  shader_program_t* prog = &rect_shader->program;
  if ((prog->valid =
           procy_compile_vert_shader((char*)embed_rect_vert, &prog->vertex) &&
           procy_compile_frag_shader((char*)embed_rect_frag,
                                     &prog->fragment))) {
    GL_CHECK(glGenVertexArrays(1, &prog->vao));
    GL_CHECK(glBindVertexArray(prog->vao));

    prog->vbo_count = 2;
    prog->vbo = malloc(sizeof(GLuint) * prog->vbo_count);
    GL_CHECK(glGenBuffers(prog->vbo_count, prog->vbo));

    prog->valid &=
        procy_link_shader_program(prog->vertex, prog->fragment, &prog->program);

    if (prog->valid) {
      rect_shader->u_ortho = glGetUniformLocation(prog->program, "u_Ortho");
    }
  }

  return rect_shader;
}

void procy_destroy_rect_shader(rect_shader_program_t* shader) {
  if (shader != NULL) {
    procy_destroy_shader_program(&shader->program);

    if (shader->vertex_buffer != NULL) {
      free(shader->vertex_buffer);
    }

    if (shader->index_buffer != NULL) {
      free(shader->index_buffer);
    }

    free(shader);
  }
}

void procy_draw_rect_shader(rect_shader_program_t* shader,
                            struct procy_window_t* window) {
  size_t ops_count = window->draw_ops.length;
  draw_op_t* ops_buffer = window->draw_ops.buffer;
  size_t rect_count = count_rects_in_ops_buffer(ops_buffer, ops_count);
  if (rect_count == 0) {
    return;
  }

  update_buffer_sizes(shader, rect_count);

  rect_vertex_t* vertices = (rect_vertex_t*)shader->vertex_buffer;
  GLushort* indices = (GLushort*)shader->index_buffer;

  size_t rect_index = 0;
  for (size_t i = 0; i < ops_count; ++i) {
    draw_op_t* op = &ops_buffer[i];
    if (op->type != DRAW_OP_RECT) {
      continue;
    }

    size_t vert_ix = rect_index * VERTICES_PER_RECT;
    size_t index_ix = rect_index * INDICES_PER_RECT;

    float x = (float)op->x;
    float y = (float)op->y;
    float w = (float)op->data.rect.width;
    float h = (float)op->data.rect.height;

    rect_vertex_t top_left = {x, y, op->forecolor};
    rect_vertex_t top_right = {x + w, y, op->forecolor};
    rect_vertex_t bottom_left = {x, y + h, op->forecolor};
    rect_vertex_t bottom_right = {x + w, y + h, op->forecolor};

    vertices[vert_ix] = top_left;
    vertices[vert_ix + 1] = top_right;
    vertices[vert_ix + 2] = bottom_left;
    vertices[vert_ix + 3] = bottom_right;

    indices[index_ix] = vert_ix;
    indices[index_ix + 1] = vert_ix + 1;
    indices[index_ix + 2] = vert_ix + 2;
    indices[index_ix + 3] = vert_ix + 1;
    indices[index_ix + 4] = vert_ix + 3;
    indices[index_ix + 5] = vert_ix + 2;

    ++rect_index;
  }

  shader_program_t* prog = &shader->program;
  GL_CHECK(glUseProgram(prog->program));

  GL_CHECK(
      glUniformMatrix4fv(shader->u_ortho, 1, GL_FALSE, &window->ortho[0][0]));

  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, prog->vbo[VBO_RECT_POSITION]));
  GL_CHECK(glBufferData(GL_ARRAY_BUFFER,
                        rect_count * VERTICES_PER_RECT * sizeof(rect_vertex_t),
                        vertices, GL_STATIC_DRAW));

  GL_CHECK(glEnableVertexAttribArray(ATTR_RECT_POSITION));
  GL_CHECK(glVertexAttribPointer(ATTR_RECT_POSITION, 2, GL_FLOAT, GL_FALSE,
                                 sizeof(rect_vertex_t), 0));

  GL_CHECK(glEnableVertexAttribArray(ATTR_RECT_COLOR));
  GL_CHECK(glVertexAttribPointer(ATTR_RECT_COLOR, 3, GL_FLOAT, GL_FALSE,
                                 sizeof(rect_vertex_t),
                                 (void*)(2 * sizeof(float))));

  GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prog->vbo[VBO_RECT_INDICES]));
  GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                        rect_count * INDICES_PER_RECT * sizeof(GLushort),
                        indices, GL_STATIC_DRAW));

  GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

  GL_CHECK(glDrawElements(GL_TRIANGLES, rect_count * INDICES_PER_RECT,
                          GL_UNSIGNED_SHORT, 0));

  glDisableVertexAttribArray(ATTR_RECT_POSITION);
  glDisableVertexAttribArray(ATTR_RECT_COLOR);
  glUseProgram(0);
}

