#include "shader/sprite.h"

#include <errno.h>
#include <log.h>
#include <stb_ds.h>
#include <string.h>

#define STBI_FAILURE_USERMSG
#include <stb_image.h>

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "drawing.h"
#include "gen/sprite_frag.h"
#include "gen/sprite_vert.h"
#include "shader/error.h"
#include "window.h"

typedef procy_window_t window_t;
typedef procy_shader_program_t shader_program_t;
typedef procy_sprite_shader_program_t sprite_shader_program_t;
typedef procy_color_t color_t;
typedef procy_sprite_t sprite_t;
typedef procy_draw_op_sprite_t draw_op_sprite_t;

#pragma pack(0)
typedef struct sprite_vertex_t {
  float x, y, z, u, v;
  int forecolor, backcolor;
} sprite_vertex_t;
#pragma pack(1)

static const size_t VBO_SPRITE_POSITION = 0;
static const size_t VBO_SPRITE_INDICES = 1;
static const size_t ATTR_SPRITE_POSITION = 0;
static const size_t ATTR_SPRITE_TEXCOORDS = 1;
static const size_t ATTR_SPRITE_FORECOLOR = 2;
static const size_t ATTR_SPRITE_BACKCOLOR = 3;

#define VERTICES_PER_SPRITE 4
#define INDICES_PER_SPRITE 6
#define DRAW_BATCH_SIZE 4096

static void enable_shader_attributes(shader_program_t *const program) {
  GL_CHECK(glBindVertexArray(program->vao));
  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, program->vbo[VBO_SPRITE_POSITION]));

  GL_CHECK(glEnableVertexAttribArray(ATTR_SPRITE_POSITION));
  GL_CHECK(glVertexAttribPointer(ATTR_SPRITE_POSITION, 3, GL_FLOAT, GL_FALSE,
                                 sizeof(sprite_vertex_t), 0));

  GL_CHECK(glEnableVertexAttribArray(ATTR_SPRITE_TEXCOORDS));
  GL_CHECK(glVertexAttribPointer(ATTR_SPRITE_TEXCOORDS, 2, GL_FLOAT, GL_FALSE,
                                 sizeof(sprite_vertex_t),
                                 (void *)(3 * sizeof(float))));  // NOLINT

  GL_CHECK(glEnableVertexAttribArray(ATTR_SPRITE_FORECOLOR));
  GL_CHECK(glVertexAttribIPointer(ATTR_SPRITE_FORECOLOR, 1, GL_INT,
                                  sizeof(sprite_vertex_t),
                                  (void *)(5 * sizeof(float))));  // NOLINT

  GL_CHECK(glEnableVertexAttribArray(ATTR_SPRITE_BACKCOLOR));
  GL_CHECK(glVertexAttribIPointer(
      ATTR_SPRITE_BACKCOLOR, 1, GL_INT, sizeof(sprite_vertex_t),
      (void *)(5 * sizeof(float) + sizeof(int))));  // NOLINT
}

static void disable_shader_attributes() {
  glDisableVertexAttribArray(ATTR_SPRITE_POSITION);
  glDisableVertexAttribArray(ATTR_SPRITE_TEXCOORDS);
  glDisableVertexAttribArray(ATTR_SPRITE_FORECOLOR);
  glDisableVertexAttribArray(ATTR_SPRITE_BACKCOLOR);
}

static bool load_sprite_texture(sprite_shader_program_t *shader,
                                unsigned char *contents, size_t length) {
  if (glIsTexture(shader->texture)) {
    GL_CHECK(glDeleteTextures(1, &shader->texture));
  }

  int components;
  unsigned char *bitmap =
      stbi_load_from_memory(contents, (int)length, &shader->texture_w,
                            &shader->texture_h, &components, 0);

  if (bitmap == NULL || shader->texture_w < 0 || shader->texture_h < 0) {
    const char *msg = stbi_failure_reason();
    if (msg != NULL) {
      log_error("STBI failure message: %s", msg);
    }

    return false;
  }

  log_debug("Loaded texture (size: %dx%d; comp: %d)", shader->texture_w,
            shader->texture_h, components);
  // copy both bitmap buffers into a single location
  const size_t bitmap_size = (size_t)shader->texture_w * shader->texture_h;
  if (bitmap_size != 0) {
    // create font texture array from bitmaps
    GL_CHECK(glGenTextures(1, &shader->texture));

    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, shader->texture));

    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, shader->texture_w,
                          shader->texture_h, 0, GL_RED, GL_UNSIGNED_BYTE,
                          bitmap));

    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
  } else {
    log_error("Sprite texture size is zero!");
    return false;
  }

  if (bitmap != NULL) {
    stbi_image_free(bitmap);
  }

  return true;
}

static void draw_sprite_batch(shader_program_t *const program,
                              sprite_vertex_t *const vertices,
                              GLushort *const indices, size_t sprite_count) {
  int buffer_size;

  // copy vertex data to video memory
  const size_t vertex_buffer_size =
      sprite_count * VERTICES_PER_SPRITE * sizeof(sprite_vertex_t);
  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, program->vbo[VBO_SPRITE_POSITION]));
  GL_CHECK(
      glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &buffer_size));
  if (buffer_size == vertex_buffer_size) {
    GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_buffer_size, vertices));
  } else {
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, vertices,
                          GL_STATIC_DRAW));
  }

  // copy indices
  const size_t index_buffer_size =
      sprite_count * INDICES_PER_SPRITE * sizeof(GLushort);
  GL_CHECK(
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, program->vbo[VBO_SPRITE_INDICES]));
  GL_CHECK(glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE,
                                  &buffer_size));
  if (buffer_size == index_buffer_size) {
    GL_CHECK(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, index_buffer_size,
                             indices));
  } else {
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                          sprite_count * INDICES_PER_SPRITE * sizeof(GLushort),
                          indices, GL_STATIC_DRAW));
  }

  // make draw call
  GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
  GL_CHECK(glDrawElements(GL_TRIANGLES, (int)sprite_count * INDICES_PER_SPRITE,
                          GL_UNSIGNED_SHORT, 0));
}

sprite_shader_program_t *procy_create_sprite_shader_mem(unsigned char *contents,
                                                        size_t length) {
  sprite_shader_program_t *sprite_shader =
      malloc(sizeof(sprite_shader_program_t));

  if (sprite_shader == NULL) {
    log_error("Failed to allocate memory for the sprite shader");
    return NULL;
  }

  sprite_shader->index_batch_buffer =
      malloc(sizeof(GLushort) * DRAW_BATCH_SIZE * INDICES_PER_SPRITE);
  sprite_shader->vertex_batch_buffer =
      malloc(sizeof(sprite_vertex_t) * DRAW_BATCH_SIZE * VERTICES_PER_SPRITE);
  sprite_shader->texture = 0;
  sprite_shader->texture_w = -1;
  sprite_shader->texture_h = -1;

  shader_program_t *program = &sprite_shader->program;
  memset(program, 0, sizeof(shader_program_t));
  if ((program->valid = procy_compile_vert_shader((char *)embed_sprite_vert,
                                                  &program->vertex) &&
                        procy_compile_frag_shader((char *)embed_sprite_frag,
                                                  &program->fragment))) {
    // load font texture and codepoints
    if (!load_sprite_texture(sprite_shader, contents, length)) {
      procy_destroy_sprite_shader(sprite_shader);
      return NULL;
    }

    // create vertex array
    GL_CHECK(glGenVertexArrays(1, &program->vao));
    GL_CHECK(glBindVertexArray(program->vao));

    // create vertex buffers
    program->vbo_count = 2;
    program->vbo = malloc(sizeof(GLuint) * program->vbo_count);
    GL_CHECK(glGenBuffers((int)program->vbo_count, program->vbo));

    program->valid &= procy_link_shader_program(
        program->vertex, program->fragment, &program->program);

    if (program->valid) {
      sprite_shader->u_ortho =
          GL_CHECK(glGetUniformLocation(program->program, "u_Ortho"));
    }
  }

  return sprite_shader;
}

sprite_shader_program_t *procy_create_sprite_shader(const char *path) {
  FILE *file = fopen(path, "rb");
  if (file == NULL) {
    log_error("Failed to open sprite texture file at \"%s\": %s", path,
              strerror(errno));
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  size_t len = ftell(file);
  unsigned char *buffer = malloc(sizeof(unsigned char) * len);
  if (buffer == NULL) {
    log_error("Failed to allocate %zu bytes of memory for \"%s\"", len, path);
    fclose(file);
    return NULL;
  }

  fseek(file, 0, SEEK_SET);
  fread(buffer, sizeof(unsigned char), len, file);
  fclose(file);

  sprite_shader_program_t *shader = procy_create_sprite_shader_mem(buffer, len);

  free(buffer);

  return shader;
}

static void compute_sprite_vertices(sprite_shader_program_t *shader,
                                    procy_draw_op_sprite_t *const op,
                                    sprite_vertex_t *const vertices,
                                    float scale) {
  sprite_t *sprite = op->ptr;

  // screen coordinates
  float x = (float)op->x;
  float y = (float)op->y;
  float z = (float)op->z;
  float width = (float)sprite->width;
  float height = (float)sprite->height;

  // texture coordinates
  float tx = (float)sprite->x / (float)shader->texture_w;
  float ty = (float)sprite->y / (float)shader->texture_h;
  float tw = width / (float)shader->texture_w;
  float th = height / (float)shader->texture_h;
  int fg = op->color.value;
  int bg = op->background.value;

  vertices[0] = (sprite_vertex_t){x, y, z, tx, ty, fg, bg};
  vertices[1] = (sprite_vertex_t){x + width * scale, y, z, tx + tw, ty, fg, bg};
  vertices[2] =
      (sprite_vertex_t){x, y + height * scale, z, tx, ty + th, fg, bg};
  vertices[3] = (sprite_vertex_t){
      x + width * scale, y + height * scale, z, tx + tw, ty + th, fg, bg};
}

void procy_draw_sprite_shader(procy_sprite_shader_program_t *shader,
                              window_t *window,
                              struct procy_draw_op_sprite_t *draw_ops) {
  sprite_vertex_t *vertex_batch = shader->vertex_batch_buffer;
  GLushort *index_batch = shader->index_batch_buffer;

  shader_program_t *program = &shader->program;
  GL_CHECK(glUseProgram(program->program));

  GL_CHECK(glActiveTexture(GL_TEXTURE0));
  GL_CHECK(glBindTexture(GL_TEXTURE_2D, shader->texture));

  // set orthographic projection matrix
  GL_CHECK(
      glUniformMatrix4fv(shader->u_ortho, 1, GL_FALSE, &window->ortho[0][0]));

  enable_shader_attributes(program);

  long batch_index = -1;
  while (arrlen(draw_ops) > 0) {
    draw_op_sprite_t op = arrpop(draw_ops);

    ++batch_index;

    const size_t vert_index = (size_t)batch_index * VERTICES_PER_SPRITE;

    // compute the sprite's 4 vertices
    sprite_vertex_t temp_vertex_buffer[VERTICES_PER_SPRITE];
    compute_sprite_vertices(shader, &op, &temp_vertex_buffer[0], window->scale);

    // specify the indices of the vertices in the order they're to be drawn
    const GLushort temp_index_buffer[] = {vert_index,     vert_index + 1,
                                          vert_index + 2, vert_index + 1,
                                          vert_index + 3, vert_index + 2};

    // copy vertices and indices to the batch buffer
    memcpy(&vertex_batch[vert_index], temp_vertex_buffer,
           sizeof(temp_vertex_buffer));
    memcpy(&index_batch[batch_index * INDICES_PER_SPRITE], temp_index_buffer,
           sizeof(temp_index_buffer));

    // if we've reached the end of the current batch, draw it and reset the
    // index
    if (batch_index == DRAW_BATCH_SIZE - 1) {
      draw_sprite_batch(program, vertex_batch, index_batch, batch_index + 1);
      batch_index = -1;
    }
  }

  // if there are any remaining sprites in the batch buffer, draw them
  if (batch_index >= 0) {
    draw_sprite_batch(program, &vertex_batch[0], &index_batch[0],
                      batch_index + 1);
  }

  disable_shader_attributes();

  glUseProgram(0);
}

void procy_destroy_sprite_shader(sprite_shader_program_t *shader) {
  if (shader != NULL) {
    procy_destroy_shader_program(&shader->program);

    // delete font texture
    if (glIsTexture(shader->texture)) {
      glDeleteTextures(1, &shader->texture);
    }

    if (shader->vertex_batch_buffer != NULL) {
      free(shader->vertex_batch_buffer);
    }

    if (shader->index_batch_buffer != NULL) {
      free(shader->index_batch_buffer);
    }

    free(shader);
  }
}
