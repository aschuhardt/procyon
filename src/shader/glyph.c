#include "shader/glyph.h"

#include "shader.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <stb_image.h>

// clang-format off
#include "opengl.h"
#include <GLFW/glfw3.h>
// clang-format on

#include <log.h>
#include <stb_ds.h>

#include "drawing.h"
#include "gen/glyph_frag.h"
#include "gen/glyph_vert.h"
#include "gen/tileset.h"
#include "gen/tileset_bold.h"
#include "shader/error.h"
#include "window.h"

typedef procy_window_t window_t;
typedef procy_shader_program_t shader_program_t;
typedef procy_glyph_shader_program_t glyph_shader_program_t;
typedef procy_color_t color_t;
typedef procy_draw_op_text_t draw_op_text_t;

#pragma pack(0)
typedef struct glyph_vertex_t {
  float x, y, z, u, v;
  int forecolor;
  int backcolor;
  float bold;
} glyph_vertex_t;
#pragma pack(1)

#define VBO_GLYPH_POSITION 0
#define VBO_GLYPH_INDICES 1
#define ATTR_GLYPH_POSITION 0
#define ATTR_GLYPH_TEXCOORDS 1
#define ATTR_GLYPH_FORECOLOR 2
#define ATTR_GLYPH_BACKCOLOR 3
#define ATTR_GLYPH_BOLD 4

// size of the glyph texture in terms of number of glyphs per side
#define GLYPH_WIDTH_COUNT 16
#define GLYPH_HEIGHT_COUNT 16

#define VERTICES_PER_GLYPH 4
#define INDICES_PER_GLYPH 6
#define DRAW_BATCH_SIZE 4096

static void enable_shader_attributes(shader_program_t *program) {
  GL_CHECK(glBindVertexArray(program->vao));
  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, program->vbo[VBO_GLYPH_POSITION]));

  GL_CHECK(glEnableVertexAttribArray(ATTR_GLYPH_POSITION));
  GL_CHECK(glVertexAttribPointer(ATTR_GLYPH_POSITION, 3, GL_FLOAT, GL_FALSE,
                                 sizeof(glyph_vertex_t), 0));

  GL_CHECK(glEnableVertexAttribArray(ATTR_GLYPH_TEXCOORDS));
  GL_CHECK(glVertexAttribPointer(ATTR_GLYPH_TEXCOORDS, 2, GL_FLOAT, GL_FALSE,
                                 sizeof(glyph_vertex_t),
                                 (void *)(3 * sizeof(float))));  // NOLINT

  GL_CHECK(glEnableVertexAttribArray(ATTR_GLYPH_FORECOLOR));
  GL_CHECK(glVertexAttribIPointer(ATTR_GLYPH_FORECOLOR, 1, GL_INT,
                                  sizeof(glyph_vertex_t),
                                  (void *)(5 * sizeof(float))));  // NOLINT

  GL_CHECK(glEnableVertexAttribArray(ATTR_GLYPH_BACKCOLOR));
  GL_CHECK(glVertexAttribIPointer(
      ATTR_GLYPH_BACKCOLOR, 1, GL_INT, sizeof(glyph_vertex_t),
      (void *)(5 * sizeof(float) + sizeof(int))));  // NOLINT

  GL_CHECK(glEnableVertexAttribArray(ATTR_GLYPH_BOLD));
  GL_CHECK(glVertexAttribPointer(
      ATTR_GLYPH_BOLD, 1, GL_FLOAT, GL_FALSE, sizeof(glyph_vertex_t),
      (void *)(5 * sizeof(float) + 2 * sizeof(int))));  // NOLINT
}

static void load_glyph_font(glyph_shader_program_t *shader) {
  if (glIsTexture(shader->font_texture)) {
    GL_CHECK(glDeleteTextures(1, &shader->font_texture));
  }

  int components;
  unsigned char *bitmap_thin = stbi_load_from_memory(
      embed_tileset, sizeof(embed_tileset) / sizeof(unsigned char),
      &shader->texture_bounds.width, &shader->texture_bounds.height,
      &components, 1);
  unsigned char *bitmap_bold = stbi_load_from_memory(
      embed_tileset_bold, sizeof(embed_tileset_bold) / sizeof(unsigned char),
      &shader->texture_bounds.width, &shader->texture_bounds.height,
      &components, 1);

  if (bitmap_thin == NULL || bitmap_bold == NULL ||
      shader->texture_bounds.width < 0 || shader->texture_bounds.height < 0) {
    log_error("Failed to read glyph texture");
    const char *msg = stbi_failure_reason();
    if (msg != NULL) {
      log_error("STBI failure message: %s", msg);
    }
  } else {
    // compute glyph screen- and texture-space bounds
    shader->glyph_bounds.width =
        shader->texture_bounds.width / GLYPH_WIDTH_COUNT;
    shader->glyph_bounds.height =
        shader->texture_bounds.height / GLYPH_HEIGHT_COUNT;
    shader->glyph_bounds.tex_width =
        (float)shader->glyph_bounds.width / (float)shader->texture_bounds.width;
    shader->glyph_bounds.tex_height = (float)shader->glyph_bounds.height /
                                      (float)shader->texture_bounds.height;

    // copy both bitmap buffers into a single location
    size_t bitmap_size =
        (size_t)shader->texture_bounds.width * shader->texture_bounds.height;
    if (bitmap_size != 0) {
      unsigned char *combined_buffer =
          malloc(sizeof(unsigned char) * bitmap_size * 2);
      if (combined_buffer != NULL) {
        memcpy(combined_buffer, bitmap_thin, bitmap_size);
        memcpy(&combined_buffer[bitmap_size], bitmap_bold, bitmap_size);

        // create font texture array from bitmaps
        GL_CHECK(glGenTextures(1, &shader->font_texture));

        GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, shader->font_texture));
        GL_CHECK(glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RED,
                              shader->texture_bounds.width,
                              shader->texture_bounds.height, 2, 0, GL_RED,
                              GL_UNSIGNED_BYTE, combined_buffer));

        free(combined_buffer);

        GL_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER,
                                 GL_NEAREST));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER,
                                 GL_NEAREST));

        log_debug("Combined glyph bitmap size: %zu", bitmap_size * 2);
        log_debug("Glyph texture ID: %u", shader->font_texture);
      } else {
        log_error(
            "Failed to allocate enough memory for combined glyph texture");
      }
    } else {
      log_error("Glyph texture size is zero!");
    }
  }

  if (bitmap_thin != NULL) {
    stbi_image_free(bitmap_thin);
  }

  if (bitmap_bold != NULL) {
    stbi_image_free(bitmap_bold);
  }
}

static void draw_glyph_batch(shader_program_t *program,
                             glyph_vertex_t *vertices, unsigned short *indices,
                             size_t glyph_count) {
  int buffer_size;

  // copy vertex data to video memory
  size_t vertex_buffer_size =
      glyph_count * VERTICES_PER_GLYPH * sizeof(glyph_vertex_t);
  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, program->vbo[VBO_GLYPH_POSITION]));
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
      glyph_count * INDICES_PER_GLYPH * sizeof(unsigned short);
  GL_CHECK(
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, program->vbo[VBO_GLYPH_INDICES]));
  GL_CHECK(glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE,
                                  &buffer_size));
  if (buffer_size == index_buffer_size) {
    GL_CHECK(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, index_buffer_size,
                             indices));
  } else {
    GL_CHECK(
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     glyph_count * INDICES_PER_GLYPH * sizeof(unsigned short),
                     indices, GL_STATIC_DRAW));
  }

  // make draw call
  GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
  GL_CHECK(glDrawElements(GL_TRIANGLES, (int)glyph_count * INDICES_PER_GLYPH,
                          GL_UNSIGNED_SHORT, 0));
}

glyph_shader_program_t *procy_create_glyph_shader() {
  glyph_shader_program_t *shader = calloc(1, sizeof(glyph_shader_program_t));

  shader->index_batch_buffer =
      malloc(sizeof(unsigned short) * DRAW_BATCH_SIZE * INDICES_PER_GLYPH);
  shader->vertex_batch_buffer =
      malloc(sizeof(glyph_vertex_t) * DRAW_BATCH_SIZE * VERTICES_PER_GLYPH);

  shader_program_t *program = &shader->program;

  // load font texture and codepoints
  load_glyph_font(shader);

  if (procy_compile_and_link_shader(program, (char *)&embed_glyph_vert[0],
                                    (char *)&embed_glyph_frag[0])) {
    shader->u_ortho =
        GL_CHECK(glGetUniformLocation(program->program, "u_Ortho"));
  }

  // create vertex array
  GL_CHECK(glGenVertexArrays(1, &program->vao));
  GL_CHECK(glBindVertexArray(program->vao));

  // create vertex buffers
  program->vbo_count = 2;
  program->vbo = malloc(sizeof(GLuint) * program->vbo_count);
  GL_CHECK(glGenBuffers((int)program->vbo_count, program->vbo));

  return shader;
}

static void compute_glyph_vertices(glyph_shader_program_t *shader,
                                   draw_op_text_t *op, glyph_vertex_t *vertices,
                                   window_t *window) {
  unsigned char c = op->character;

  float gw = (float)shader->glyph_bounds.width;
  float gh = (float)shader->glyph_bounds.height;
  float tw = shader->glyph_bounds.tex_width;
  float th = shader->glyph_bounds.tex_height;

  // screen coordinates
  float x = (float)op->x;
  float y = (float)op->y;
  float z = (float)op->z;

  // texture coordinates
  float tx = (float)(c % GLYPH_WIDTH_COUNT) * (float)gw /
             (float)shader->texture_bounds.width;
  float ty = floorf((float)c / (float)GLYPH_HEIGHT_COUNT) * (float)gh /
             (float)shader->texture_bounds.height;

  // colors + bold
  int fg = op->color.value;
  int bg = op->background.value;
  float bold = op->bold ? 1.0F : 0.0F;

  vertices[0] = (glyph_vertex_t){x, y, z, tx, ty, fg, bg, bold};
  vertices[1] = (glyph_vertex_t){x + gw, y, z, tx + tw, ty, fg, bg, bold};
  vertices[2] = (glyph_vertex_t){x, y + gh, z, tx, ty + th, fg, bg, bold};
  vertices[3] =
      (glyph_vertex_t){x + gw, y + gh, z, tx + tw, ty + th, fg, bg, bold};
}

void procy_draw_glyph_shader(glyph_shader_program_t *shader, window_t *window,
                             draw_op_text_t *draw_ops) {
  glyph_vertex_t *vertex_batch = shader->vertex_batch_buffer;
  unsigned short *index_batch = shader->index_batch_buffer;

  shader_program_t *program = &shader->program;
  GL_CHECK(glUseProgram(program->program));

  GL_CHECK(glActiveTexture(GL_TEXTURE0));
  GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, shader->font_texture));

  // set orthographic projection matrix
  GL_CHECK(
      glUniformMatrix4fv(shader->u_ortho, 1, GL_FALSE, &window->ortho[0][0]));

  enable_shader_attributes(program);

  // glyph vertex dimensions and u/v coordinates will be the same for every
  // glyph drawn, so compute them just once

  long batch_index = -1;
  while (arrlen(draw_ops) > 0) {
    draw_op_text_t op = arrpop(draw_ops);

    ++batch_index;

    size_t vert_index = (size_t)batch_index * VERTICES_PER_GLYPH;

    // compute the glyph's 4 vertices
    glyph_vertex_t temp_vertex_buffer[VERTICES_PER_GLYPH];
    compute_glyph_vertices(shader, &op, &temp_vertex_buffer[0], window);

    // specify the indices of the vertices in the order they're to be drawn
    unsigned short temp_index_buffer[] = {vert_index,     vert_index + 1,
                                        vert_index + 2, vert_index + 1,
                                        vert_index + 3, vert_index + 2};

    // copy vertices and indices to the batch buffer
    memcpy(&vertex_batch[vert_index], temp_vertex_buffer,
           sizeof(temp_vertex_buffer));
    memcpy(&index_batch[batch_index * INDICES_PER_GLYPH], temp_index_buffer,
           sizeof(temp_index_buffer));

    // if we've reached the end of the current batch, draw it and reset the
    // index
    if (batch_index == DRAW_BATCH_SIZE - 1) {
      draw_glyph_batch(program, vertex_batch, index_batch, batch_index + 1);
      batch_index = -1;
    }
  }

  // if there are any remaining glyphs in the batch buffer, draw them
  if (batch_index >= 0) {
    draw_glyph_batch(program, &vertex_batch[0], &index_batch[0],
                     batch_index + 1);
  }

  glUseProgram(0);
}

void procy_get_glyph_bounds(glyph_shader_program_t *shader, int *width,
                            int *height) {
  if (width != NULL) {
    *width = shader->glyph_bounds.width;
  }
  if (height != NULL) {
    *height = shader->glyph_bounds.height;
  }
}

void procy_destroy_glyph_shader(glyph_shader_program_t *shader) {
  if (shader != NULL) {
    procy_destroy_shader_program(&shader->program);

    // delete font texture
    if (glIsTexture(shader->font_texture)) {
      glDeleteTextures(1, &shader->font_texture);
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
