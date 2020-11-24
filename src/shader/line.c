#include "shader/line.h"

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "drawing.h"
#include "window.h"
#include "gen/line_frag.h"
#include "gen/line_vert.h"
#include "shader/error.h"

#include <log.h>

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

#define VERTICES_PER_LINE 2

#define DRAW_BATCH_SIZE 4096

/* --------------------------- */
/* Public interface definition */
/* --------------------------- */

line_shader_program_t *procy_create_line_shader() {
  line_shader_program_t *line_shader = malloc(sizeof(line_shader_program_t));

  if (line_shader == NULL) {
    log_error("Failed to allocate memory for line shader");
    return NULL;
  }

  line_shader->vertex_batch_buffer =
      malloc(sizeof(line_vertex_t) * DRAW_BATCH_SIZE * VERTICES_PER_LINE);

  shader_program_t *program = &line_shader->program;
  if ((program->valid = procy_compile_vert_shader((char *)embed_line_vert,
                                                  &program->vertex) &&
                        procy_compile_frag_shader((char *)embed_line_frag,
                                                  &program->fragment))) {
    GL_CHECK(glGenVertexArrays(1, &program->vao));
    GL_CHECK(glBindVertexArray(program->vao));

    program->vbo_count = 1;
    program->vbo = malloc(sizeof(GLuint) * program->vbo_count);
    GL_CHECK(glGenBuffers((int)program->vbo_count, program->vbo));

    program->valid &= procy_link_shader_program(
        program->vertex, program->fragment, &program->program);

    if (program->valid) {
      line_shader->u_ortho = glGetUniformLocation(program->program, "u_Ortho");
    }
  }

  return line_shader;
}

void procy_destroy_line_shader(line_shader_program_t *shader) {
  if (shader != NULL) {
    procy_destroy_shader_program(&shader->program);

    if (shader->vertex_batch_buffer != NULL) {
      free(shader->vertex_batch_buffer);
    }

    free(shader);
  }
}

static void enable_shader_attributes(shader_program_t *program) {
  GL_CHECK(glBindVertexArray(program->vao));
  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, program->vbo[VBO_LINE_POSITION]));

  GL_CHECK(glEnableVertexAttribArray(ATTR_LINE_POSITION));
  GL_CHECK(glVertexAttribPointer(ATTR_LINE_POSITION, 2, GL_FLOAT, GL_FALSE,
                                 sizeof(line_vertex_t), 0));

  GL_CHECK(glEnableVertexAttribArray(ATTR_LINE_COLOR));
  GL_CHECK(glVertexAttribPointer(ATTR_LINE_COLOR, 3, GL_FLOAT, GL_FALSE,
                                 sizeof(line_vertex_t),
                                 (void *)(2 * sizeof(float))));
}

static void draw_line_batch(shader_program_t *const program,
                            line_vertex_t *const vertices, long line_count) {
  int buffer_size;

  const size_t vertex_buffer_size =
      line_count * VERTICES_PER_LINE * sizeof(line_vertex_t);
  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, program->vbo[VBO_LINE_POSITION]));
  GL_CHECK(
      glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &buffer_size));
  if (buffer_size == vertex_buffer_size) {
    GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_buffer_size, vertices));
  } else {
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, vertices,
                          GL_STATIC_DRAW));
  }

  GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
  GL_CHECK(glDrawArrays(GL_LINES, 0, line_count * VERTICES_PER_LINE));
}

void procy_draw_line_shader(line_shader_program_t *shader,
                            struct procy_window_t *window) {
  draw_op_t *ops_buffer = window->draw_ops.buffer;
  line_vertex_t *vertex_batch = shader->vertex_batch_buffer;

  shader_program_t *program = &shader->program;
  GL_CHECK(glUseProgram(program->program));

  GL_CHECK(
      glUniformMatrix4fv(shader->u_ortho, 1, GL_FALSE, &window->ortho[0][0]));

  enable_shader_attributes(program);

  long batch_index = -1;
  for (size_t i = 0; i < window->draw_ops.length; ++i) {
    draw_op_t *op = &ops_buffer[i];
    if (op->type != DRAW_OP_LINE) {
      continue;
    }

    ++batch_index;

    const size_t vert_index = (size_t)batch_index * VERTICES_PER_LINE;

    vertex_batch[vert_index] =
        (line_vertex_t){(float)op->x, (float)op->y, op->forecolor};
    vertex_batch[vert_index + 1] = (line_vertex_t){
        (float)op->data.line.x2, (float)op->data.line.y2, op->forecolor};

    if (batch_index == DRAW_BATCH_SIZE - 1) {
      draw_line_batch(program, vertex_batch, batch_index + 1);
      batch_index = -1;
    }
  }

  if (batch_index >= 0) {
    draw_line_batch(program, vertex_batch, batch_index + 1);
  }

  glDisableVertexAttribArray(ATTR_LINE_POSITION);
  glDisableVertexAttribArray(ATTR_LINE_COLOR);
  glUseProgram(0);
}
