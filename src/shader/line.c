#include "shader/line.h"

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "drawing.h"
#include "window.h"
#include "gen/line_frag.h"
#include "gen/line_vert.h"

typedef procy_line_shader_program_t line_shader_program_t;
typedef procy_window_t window_t;
typedef procy_color_t color_t;
typedef procy_shader_program_t shader_program_t;
typedef procy_draw_op_t draw_op_t;

#pragma pack(0)
typedef struct line_vertex_t {
  float x, y;
  color_t color;
} line_vertex_t;
#pragma pack(1)

static const size_t VBO_LINE_POSITION = 0;
static const size_t ATTR_LINE_POSITION = 0;
static const size_t ATTR_LINE_COLOR = 1;

static size_t count_lines_in_ops_buffer(draw_op_t* ops, size_t n) {
  size_t count = 0;
  for (int i = 0; i < n; i++) {
    if (ops[i].type == DRAW_OP_LINE) {
      ++count;
    }
  }

  return count;
}

line_shader_program_t procy_create_line_shader() {
  line_shader_program_t line_shader;

  shader_program_t* prog = &line_shader.program;
  if ((prog->valid =
           procy_compile_vert_shader((char*)embed_line_vert, &prog->vertex) &&
           procy_compile_frag_shader((char*)embed_line_frag,
                                     &prog->fragment))) {
    glGenVertexArrays(1, &prog->vao);
    glBindVertexArray(prog->vao);

    prog->vbo_count = 1;
    prog->vbo = malloc(sizeof(GLuint) * prog->vbo_count);
    glGenBuffers(prog->vbo_count, prog->vbo);

    prog->valid &=
        procy_link_shader_program(prog->vertex, prog->fragment, &prog->program);

    if (prog->valid) {
      line_shader.u_ortho = glGetUniformLocation(prog->program, "u_Ortho");
    }
  }

  return line_shader;
}

void procy_destroy_line_shader(line_shader_program_t* shader) {
  if (shader != NULL) {
    procy_destroy_shader_program(&shader->program);
  }
}

void procy_draw_line_shader(line_shader_program_t* shader,
                            struct procy_window_t* window) {
  size_t ops_count = window->draw_ops.length;
  draw_op_t* ops_buffer = window->draw_ops.buffer;
  size_t line_count = count_lines_in_ops_buffer(ops_buffer, ops_count);
  if (line_count == 0) {
    return;
  }

  line_vertex_t vertices[line_count * 2];

  size_t line_index = 0;
  for (size_t i = 0; i < ops_count; ++i) {
    draw_op_t* op = &ops_buffer[i];
    if (op->type != DRAW_OP_LINE) {
      continue;
    }

    size_t vert_ix = line_index * 2;

    float x1 = (float)op->x;
    float y1 = (float)op->y;
    float x2 = (float)op->data.line.x2;
    float y2 = (float)op->data.line.y2;

    line_vertex_t start = {x1, y1, op->forecolor};
    line_vertex_t end = {x2, y2, op->forecolor};

    vertices[vert_ix] = start;
    vertices[vert_ix + 1] = end;

    ++line_index;
  }

  shader_program_t* prog = &shader->program;
  glUseProgram(prog->program);

  glUniformMatrix4fv(shader->u_ortho, 1, GL_FALSE, &window->ortho[0][0]);

  glBindBuffer(GL_ARRAY_BUFFER, prog->vbo[VBO_LINE_POSITION]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(ATTR_LINE_POSITION);
  glVertexAttribPointer(ATTR_LINE_POSITION, 2, GL_FLOAT, GL_FALSE,
                        sizeof(line_vertex_t), 0);

  glEnableVertexAttribArray(ATTR_LINE_COLOR);
  glVertexAttribPointer(ATTR_LINE_COLOR, 3, GL_FLOAT, GL_FALSE,
                        sizeof(line_vertex_t), (void*)(2 * sizeof(float)));

  glPolygonMode(GL_FRONT, GL_LINE);

  glDrawArrays(GL_LINES, 0, sizeof(vertices) / sizeof(line_vertex_t));

  glDisableVertexAttribArray(ATTR_LINE_POSITION);
  glDisableVertexAttribArray(ATTR_LINE_COLOR);
  glUseProgram(0);
}

