#include "shader/glyph.h"

#include <log.h>

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "shader/error.h"
#include "drawing.h"
#include "window.h"
#include "gen/tileset.h"
#include "gen/tileset_bold.h"
#include "gen/glyph_frag.h"
#include "gen/glyph_vert.h"

typedef procy_draw_op_t draw_op_t;
typedef procy_window_t window_t;
<<<<<<< HEAD
typedef procy_glyph_bounds_t glyph_bounds_t;
=======
>>>>>>> 5a47373a6b26524ae1a6b8c48f72e9a3f7a16059
typedef procy_shader_program_t shader_program_t;
typedef procy_glyph_shader_program_t glyph_shader_program_t;
typedef procy_color_t color_t;

#pragma pack(0)
typedef struct glyph_vertex_t {
  float x, y, u, v;
  color_t forecolor, backcolor;
  float bold;
} glyph_vertex_t;
#pragma pack(1)

static const size_t VBO_GLYPH_POSITION = 0;
static const size_t VBO_GLYPH_INDICES = 1;
static const size_t ATTR_GLYPH_POSITION = 0;
static const size_t ATTR_GLYPH_TEXCOORDS = 1;
static const size_t ATTR_GLYPH_FORECOLOR = 2;
static const size_t ATTR_GLYPH_BACKCOLOR = 3;
static const size_t ATTR_GLYPH_BOLD = 4;

// size of the glyph texture in terms of number of glyphs per side
static const size_t GLYPH_WIDTH_COUNT = 16;
static const size_t GLYPH_HEIGHT_COUNT = 16;

static const size_t VERTICES_PER_GLYPH = 4;
static const size_t INDICES_PER_GLYPH = 6;

static void load_glyph_font(glyph_shader_program_t* shader) {
  if (glIsTexture(shader->font_texture)) {
    GL_CHECK(glDeleteTextures(1, &shader->font_texture));
  }

  int components;
  unsigned char* bitmap_thin = stbi_load_from_memory(
      embed_tileset, sizeof(embed_tileset) / sizeof(unsigned char),
      &shader->texture_w, &shader->texture_h, &components, 1);
  unsigned char* bitmap_bold = stbi_load_from_memory(
      embed_tileset_bold, sizeof(embed_tileset_bold) / sizeof(unsigned char),
      &shader->texture_w, &shader->texture_h, &components, 1);

  if (bitmap_thin == NULL || bitmap_bold == NULL || shader->texture_w < 0 ||
      shader->texture_h < 0) {
    log_error("Failed to read glyph texture");
    const char* msg = stbi_failure_reason();
    if (msg != NULL) {
      log_error("STBI failure message: %s", msg);
    }
  } else {
    // copy both bitmap buffers into a single location
    size_t bitmap_size = shader->texture_w * shader->texture_h;
    unsigned char* combined_buffer =
        malloc(sizeof(unsigned char) * bitmap_size * 2);
    memcpy(combined_buffer, bitmap_thin, bitmap_size);
    memcpy(&combined_buffer[bitmap_size], bitmap_bold, bitmap_size);

<<<<<<< HEAD
=======
    // set glyph size on window so that it's accessible to the script
    // environment
    shader->window->glyph.width = (int)floorf(
        (float)shader->texture_w / GLYPH_WIDTH_COUNT * shader->scale);
    shader->window->glyph.height = (int)floorf(
        (float)shader->texture_h / GLYPH_HEIGHT_COUNT * shader->scale);

>>>>>>> 5a47373a6b26524ae1a6b8c48f72e9a3f7a16059
    // create font texture array from bitmaps
    GL_CHECK(glGenTextures(1, &shader->font_texture));

    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, shader->font_texture));

    GL_CHECK(glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RED, shader->texture_w,
                          shader->texture_h, 2, 0, GL_RED, GL_UNSIGNED_BYTE,
                          combined_buffer));

    free(combined_buffer);

<<<<<<< HEAD
=======
    /*
    GL_CHECK(glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R8, shader->texture_w,
                            shader->texture_h, 4));
    GL_CHECK(glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, shader->texture_w,
                             shader->texture_h, 2, GL_RED, GL_UNSIGNED_BYTE,
                             bitmap_thin));
    GL_CHECK(glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1, shader->texture_w,
                             shader->texture_h, 2, GL_RED, GL_UNSIGNED_BYTE,
                             bitmap_bold));
                             */
>>>>>>> 5a47373a6b26524ae1a6b8c48f72e9a3f7a16059
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER,
                             GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER,
                             GL_NEAREST));
  }

  if (bitmap_thin != NULL) {
    stbi_image_free(bitmap_thin);
  }

  if (bitmap_bold != NULL) {
    stbi_image_free(bitmap_bold);
  }
}

static size_t count_glyphs_in_ops_buffer(draw_op_t* ops, size_t n) {
  // get total number of characters
  size_t glyph_count = 0;
  for (size_t i = 0; i < n; ++i) {
    if (ops[i].type != DRAW_OP_TEXT) {
      continue;
    }

    if (ops[i].data.text.character != '\0') {
      ++glyph_count;
    }
  }

  return glyph_count;
}

static void update_buffer_sizes(glyph_shader_program_t* shader,
                                size_t glyph_count) {
  if (glyph_count != shader->glyph_count) {
    shader->index_buffer =
        realloc(shader->index_buffer,
                sizeof(GLushort) * INDICES_PER_GLYPH * glyph_count);
    shader->vertex_buffer =
        realloc(shader->vertex_buffer,
                sizeof(glyph_vertex_t) * VERTICES_PER_GLYPH * glyph_count);
    shader->glyph_count = glyph_count;
  }
}

/* --------------------------- */
/* Public interface definition */
/* --------------------------- */

<<<<<<< HEAD
glyph_shader_program_t* procy_create_glyph_shader(float scale) {
  glyph_shader_program_t* glyph_shader = malloc(sizeof(glyph_shader_program_t));

  glyph_shader->font_texture = 0;
  glyph_shader->texture_w = -1;
  glyph_shader->texture_h = -1;
  glyph_shader->glyph_scale = scale;
=======
glyph_shader_program_t* procy_create_glyph_shader(window_t* window) {
  glyph_shader_program_t* glyph_shader = malloc(sizeof(glyph_shader_program_t));

  glyph_shader->window = window;
  glyph_shader->font_texture = 0;
  glyph_shader->texture_w = -1;
  glyph_shader->texture_h = -1;
  glyph_shader->scale = window->text_scale;
>>>>>>> 5a47373a6b26524ae1a6b8c48f72e9a3f7a16059
  glyph_shader->index_buffer = NULL;
  glyph_shader->vertex_buffer = NULL;
  glyph_shader->glyph_count = 0;

  shader_program_t* prog = &glyph_shader->program;
  if ((prog->valid =
           procy_compile_vert_shader((char*)embed_glyph_vert, &prog->vertex) &&
           procy_compile_frag_shader((char*)embed_glyph_frag,
                                     &prog->fragment))) {
    // load font texture and codepoints
    load_glyph_font(glyph_shader);

    // create vertex array
    GL_CHECK(glGenVertexArrays(1, &prog->vao));
    GL_CHECK(glBindVertexArray(prog->vao));

    // create vertex buffers
    prog->vbo_count = 2;
    prog->vbo = malloc(sizeof(GLuint) * prog->vbo_count);
    GL_CHECK(glGenBuffers(prog->vbo_count, prog->vbo));

    prog->valid &=
        procy_link_shader_program(prog->vertex, prog->fragment, &prog->program);

    if (prog->valid) {
      glyph_shader->u_ortho =
          GL_CHECK(glGetUniformLocation(prog->program, "u_Ortho"));
    }
  }

  return glyph_shader;
}

void procy_draw_glyph_shader(glyph_shader_program_t* shader, window_t* window) {
  draw_op_t* ops_buffer = window->draw_ops.buffer;
  size_t glyph_count =
      count_glyphs_in_ops_buffer(ops_buffer, window->draw_ops.length);

  if (glyph_count == 0) {
    return;
  }

  // resize buffers if needed
  update_buffer_sizes(shader, glyph_count);

<<<<<<< HEAD
  float scale = shader->glyph_scale;
=======
  float scale = shader->scale;
>>>>>>> 5a47373a6b26524ae1a6b8c48f72e9a3f7a16059
  float glyph_w = (float)shader->texture_w / GLYPH_WIDTH_COUNT;
  float glyph_h = (float)shader->texture_h / GLYPH_HEIGHT_COUNT;
  float glyph_tw = glyph_w / (float)shader->texture_w;
  float glyph_th = glyph_h / (float)shader->texture_h;

  // alias data buffers to improve readability
  glyph_vertex_t* vertices = (glyph_vertex_t*)shader->vertex_buffer;
  GLushort* indices = (GLushort*)shader->index_buffer;

  size_t glyph_index = 0;
  for (size_t i = 0; i < window->draw_ops.length; ++i) {
    draw_op_t* op = &ops_buffer[i];
    if (op->type != DRAW_OP_TEXT) {
      continue;
    }

    unsigned char c = op->data.text.character;

    // screen coordinates
    float x = (float)op->x;
    float y = (float)op->y;

    // texture coordinates
    float tx =
        (float)(c % GLYPH_WIDTH_COUNT) * glyph_w / (float)shader->texture_w;
    float ty = floorf((float)c / GLYPH_HEIGHT_COUNT) * glyph_h /
               (float)shader->texture_h;

    float bold = op->data.text.bold ? 1.0 : 0.0;

    // vertex attributes
    glyph_vertex_t top_left = {x,   y, tx, ty, op->forecolor, op->backcolor,
                               bold};
    glyph_vertex_t top_right = {
        x + glyph_w * scale, y,   tx + glyph_tw, ty, op->forecolor,
        op->backcolor,       bold};
    glyph_vertex_t bottom_left = {x,
                                  y + glyph_h * scale,
                                  tx,
                                  ty + glyph_th,
                                  op->forecolor,
                                  op->backcolor,
                                  bold};
    glyph_vertex_t bottom_right = {
        x + glyph_w * scale, y + glyph_h * scale, tx + glyph_tw, ty + glyph_th,
        op->forecolor,       op->backcolor,       bold};

    size_t vert_ix = glyph_index * VERTICES_PER_GLYPH;
<<<<<<< HEAD
=======
    size_t index_ix = glyph_index * INDICES_PER_GLYPH;
    ++glyph_index;

>>>>>>> 5a47373a6b26524ae1a6b8c48f72e9a3f7a16059
    vertices[vert_ix] = top_left;
    vertices[vert_ix + 1] = top_right;
    vertices[vert_ix + 2] = bottom_left;
    vertices[vert_ix + 3] = bottom_right;

<<<<<<< HEAD
    size_t index_ix = glyph_index * INDICES_PER_GLYPH;
=======
>>>>>>> 5a47373a6b26524ae1a6b8c48f72e9a3f7a16059
    indices[index_ix] = vert_ix;
    indices[index_ix + 1] = vert_ix + 1;
    indices[index_ix + 2] = vert_ix + 2;
    indices[index_ix + 3] = vert_ix + 1;
    indices[index_ix + 4] = vert_ix + 3;
    indices[index_ix + 5] = vert_ix + 2;
<<<<<<< HEAD

    ++glyph_index;
=======
>>>>>>> 5a47373a6b26524ae1a6b8c48f72e9a3f7a16059
  }

  shader_program_t* prog = &shader->program;
  GL_CHECK(glUseProgram(prog->program));

  GL_CHECK(glActiveTexture(GL_TEXTURE0));
  GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, shader->font_texture));

  // set orthographic projection matrix
  GL_CHECK(
      glUniformMatrix4fv(shader->u_ortho, 1, GL_FALSE, &window->ortho[0][0]));

  // copy vertex data to video memory
  GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, prog->vbo[VBO_GLYPH_POSITION]));
  GL_CHECK(
      glBufferData(GL_ARRAY_BUFFER,
                   glyph_count * VERTICES_PER_GLYPH * sizeof(glyph_vertex_t),
                   vertices, GL_STATIC_DRAW));

  // associate the vertex data with the correct shader attribute
  GL_CHECK(glEnableVertexAttribArray(ATTR_GLYPH_POSITION));
  GL_CHECK(glVertexAttribPointer(ATTR_GLYPH_POSITION, 2, GL_FLOAT, GL_FALSE,
                                 sizeof(glyph_vertex_t), 0));

  GL_CHECK(glEnableVertexAttribArray(ATTR_GLYPH_TEXCOORDS));
  GL_CHECK(glVertexAttribPointer(ATTR_GLYPH_TEXCOORDS, 2, GL_FLOAT, GL_FALSE,
                                 sizeof(glyph_vertex_t),
                                 (void*)(2 * sizeof(float))));

  GL_CHECK(glEnableVertexAttribArray(ATTR_GLYPH_FORECOLOR));
  GL_CHECK(glVertexAttribPointer(ATTR_GLYPH_FORECOLOR, 3, GL_FLOAT, GL_FALSE,
                                 sizeof(glyph_vertex_t),
                                 (void*)(4 * sizeof(float))));

  GL_CHECK(glEnableVertexAttribArray(ATTR_GLYPH_BACKCOLOR));
  GL_CHECK(glVertexAttribPointer(ATTR_GLYPH_BACKCOLOR, 3, GL_FLOAT, GL_FALSE,
                                 sizeof(glyph_vertex_t),
                                 (void*)(7 * sizeof(float))));

  GL_CHECK(glEnableVertexAttribArray(ATTR_GLYPH_BOLD));
  GL_CHECK(glVertexAttribPointer(ATTR_GLYPH_BOLD, 1, GL_FLOAT, GL_FALSE,
                                 sizeof(glyph_vertex_t),
                                 (void*)(10 * sizeof(float))));

  // copy indices
  GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prog->vbo[VBO_GLYPH_INDICES]));
  GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                        glyph_count * INDICES_PER_GLYPH * sizeof(GLushort),
                        indices, GL_STATIC_DRAW));

  GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

  GL_CHECK(glDrawElements(GL_TRIANGLES, shader->glyph_count * INDICES_PER_GLYPH,
                          GL_UNSIGNED_SHORT, 0));

  glDisableVertexAttribArray(ATTR_GLYPH_POSITION);
  glDisableVertexAttribArray(ATTR_GLYPH_TEXCOORDS);
  glDisableVertexAttribArray(ATTR_GLYPH_FORECOLOR);
  glDisableVertexAttribArray(ATTR_GLYPH_BACKCOLOR);
  glDisableVertexAttribArray(ATTR_GLYPH_BOLD);
  glUseProgram(0);
}

<<<<<<< HEAD
void procy_get_glyph_bounds(glyph_shader_program_t* shader, int* width,
                            int* height) {
  *width = (float)shader->texture_w / GLYPH_WIDTH_COUNT * shader->glyph_scale;
  *height = (float)shader->texture_h / GLYPH_HEIGHT_COUNT * shader->glyph_scale;
}

=======
>>>>>>> 5a47373a6b26524ae1a6b8c48f72e9a3f7a16059
void procy_destroy_glyph_shader(glyph_shader_program_t* shader) {
  if (shader != NULL) {
    procy_destroy_shader_program(&shader->program);

    if (shader->index_buffer != NULL) {
      free(shader->index_buffer);
    }

    if (shader->vertex_buffer != NULL) {
      free(shader->vertex_buffer);
    }

    // delete font texture
    if (glIsTexture(shader->font_texture)) {
      glDeleteTextures(1, &shader->font_texture);
    }

    free(shader);
  }
}
