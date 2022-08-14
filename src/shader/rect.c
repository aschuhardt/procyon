#include "shader/rect.h"

// clang-format off
#include "opengl.h"
#include <GLFW/glfw3.h>
// clang-format on

#include <log.h>
#include <stb_ds.h>
#include <string.h>

#include "drawing.h"
#include "gen/rect_frag.h"
#include "gen/rect_vert.h"
#include "shader/error.h"
#include "window.h"

typedef procy_rect_shader_program_t rect_shader_program_t;
typedef procy_window_t window_t;
typedef procy_color_t color_t;
typedef procy_shader_program_t shader_program_t;
typedef procy_draw_op_rect_t draw_op_rect_t;

#pragma pack(0)
typedef struct rect_vertex_t {
  float x, y, z;
  int color;
} rect_vertex_t;
#pragma pack(1)

#define VBO_RECT_POSITION 0
#define VBO_RECT_INDICES 1
#define ATTR_RECT_POSITION 0
#define ATTR_RECT_COLOR 1

#define INDICES_PER_RECT 6
#define VERTICES_PER_RECT 4

#define DRAW_BATCH_SIZE 4096

rect_shader_program_t *procy_create_rect_shader(void) {
  rect_shader_program_t *rect_shader = calloc(1, sizeof(rect_shader_program_t));

  rect_shader->index_batch_buffer =
      malloc(sizeof(unsigned short) * DRAW_BATCH_SIZE * INDICES_PER_RECT);
  rect_shader->vertex_batch_buffer =
      malloc(sizeof(rect_vertex_t) * DRAW_BATCH_SIZE * VERTICES_PER_RECT);

  shader_program_t *program = &rect_shader->program;

  GL_CHECK(glGenVertexArrays(1, &program->vao));
  GL_CHECK(glBindVertexArray(program->vao));

  program->vbo_count = 2;
  program->vbo = malloc(sizeof(unsigned int) * program->vbo_count);
  GL_CHECK(glGenBuffers((int)program->vbo_count, program->vbo));

  if (procy_compile_and_link_shader(program, (char *)&embed_rect_vert[0],
                                    (char *)&embed_rect_frag[0])) {
    rect_shader->u_ortho = glGetUniformLocation(program->program, "u_Ortho");
  }

  return rect_shader;
}

void procy_destroy_rect_shader(rect_shader_program_t *shader) {
  if (shader != NULL) {
    procy_destroy_shader_program(&shader->program);

    if (shader->vertex_batch_buffer != NULL) {
      free(shader->vertex_batch_buffer);
    }

    if (shader->index_batch_buffer != NULL) {
      free(shader->index_batch_buffer);
    }

    free(shader);
  }
}

static void compute_rect_vertices(rect_shader_program_t *shader,
                                  draw_op_rect_t *op, rect_vertex_t *vertices) {
  float x = (float)op->x;
  float y = (float)op->y;
  float z = (float)op->z;
  float w = (float)op->width;
  float h = (float)op->height;
  int color = op->color.value;

  vertices[0] = (rect_vertex_t){x, y, z, color};
  vertices[1] = (rect_vertex_t){x + w, y, z, color};
  vertices[2] = (rect_vertex_t){x, y + h, z, color};
  vertices[3] = (rect_vertex_t){x + w, y + h, z, color};
}

static void draw_rect_batch(shader_program_t *program, rect_vertex_t *vertices,
                            unsigned short *indices, long rect_count) {
  int buffer_size;

  // copy vertex data to video memory
  size_t vertex_buffer_size =
      rect_count * VERTICES_PER_RECT * sizeof(rect_vertex_t);
  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, program->vbo[VBO_RECT_POSITION]));
  GL_CHECK(
      glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &buffer_size));
  if (buffer_size == vertex_buffer_size) {
    GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_buffer_size, vertices));
  } else {
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, vertices,
                          GL_STATIC_DRAW));
  }

  // copy indices
  size_t index_buffer_size =
      rect_count * INDICES_PER_RECT * sizeof(unsigned short);
  GL_CHECK(
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, program->vbo[VBO_RECT_INDICES]));
  GL_CHECK(glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE,
                                  &buffer_size));
  if (buffer_size == index_buffer_size) {
    GL_CHECK(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, index_buffer_size,
                             indices));
  } else {
    GL_CHECK(
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     rect_count * INDICES_PER_RECT * sizeof(unsigned short),
                     indices, GL_STATIC_DRAW));
  }

  // make draw call
  GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
  GL_CHECK(glDrawElements(GL_TRIANGLES, rect_count * INDICES_PER_RECT,
                          GL_UNSIGNED_SHORT, 0));
}

static void enable_shader_attributes(shader_program_t *program) {
  GL_CHECK(glBindVertexArray(program->vao));
  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, program->vbo[VBO_RECT_POSITION]));

  GL_CHECK(glEnableVertexAttribArray(ATTR_RECT_POSITION));
  GL_CHECK(glVertexAttribPointer(ATTR_RECT_POSITION, 3, GL_FLOAT, GL_FALSE,
                                 sizeof(rect_vertex_t), 0));

  GL_CHECK(glEnableVertexAttribArray(ATTR_RECT_COLOR));
  GL_CHECK(glVertexAttribIPointer(ATTR_RECT_COLOR, 1, GL_INT,
                                  sizeof(rect_vertex_t),
                                  (void *)(3 * sizeof(float))));  // NOLINT
}

void procy_draw_rect_shader(rect_shader_program_t *shader, window_t *window,
                            draw_op_rect_t *draw_ops) {
  rect_vertex_t *vertex_batch = shader->vertex_batch_buffer;
  unsigned short *index_batch = (unsigned short *)shader->index_batch_buffer;

  shader_program_t *program = &shader->program;
  GL_CHECK(glUseProgram(program->program));

  // set orthographic projection matrix
  GL_CHECK(
      glUniformMatrix4fv(shader->u_ortho, 1, GL_FALSE, &window->ortho[0][0]));

  enable_shader_attributes(program);

  long batch_index = -1;
  while (arrlen(draw_ops) > 0) {
    draw_op_rect_t op = arrpop(draw_ops);

    ++batch_index;

    size_t vert_index = (size_t)batch_index * VERTICES_PER_RECT;

    rect_vertex_t temp_vertex_buffer[VERTICES_PER_RECT];
    compute_rect_vertices(shader, &op, &temp_vertex_buffer[0]);

    unsigned short temp_index_buffer[] = {vert_index,     vert_index + 1,
                                          vert_index + 2, vert_index + 1,
                                          vert_index + 3, vert_index + 2};

    memcpy(&vertex_batch[vert_index], temp_vertex_buffer,
           sizeof(temp_vertex_buffer));
    memcpy(&index_batch[batch_index * INDICES_PER_RECT], temp_index_buffer,
           sizeof(temp_index_buffer));

    if (batch_index == DRAW_BATCH_SIZE - 1) {
      draw_rect_batch(program, vertex_batch, index_batch, batch_index + 1);
      batch_index = -1;
    }
  }

  if (batch_index >= 0) {
    draw_rect_batch(program, vertex_batch, index_batch, batch_index + 1);
  }

  glDisableVertexAttribArray(ATTR_RECT_POSITION);
  glDisableVertexAttribArray(ATTR_RECT_COLOR);
  glUseProgram(0);
}
