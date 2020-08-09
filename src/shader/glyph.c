#include "shader/glyph.h"

#include <log.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "drawing.h"
#include "window.h"
#include "gen/tileset.h"
#include "gen/glyph_frag.h"
#include "gen/glyph_vert.h"

typedef procy_draw_op_t draw_op_t;
typedef procy_window_t window_t;
typedef procy_shader_program_t shader_program_t;
typedef procy_glyph_shader_program_t glyph_shader_program_t;
typedef procy_color_t color_t;

#pragma pack(0)
typedef struct glyph_vertex_t {
  float x, y, u, v;
  color_t forecolor, backcolor;
} glyph_vertex_t;
#pragma pack(1)

static const size_t VBO_GLYPH_POSITION = 0;
static const size_t VBO_GLYPH_INDICES = 1;
static const size_t ATTR_GLYPH_POSITION = 0;
static const size_t ATTR_GLYPH_TEXCOORDS = 1;
static const size_t ATTR_GLYPH_FORECOLOR = 2;
static const size_t ATTR_GLYPH_BACKCOLOR = 3;

// size of the glyph texture in terms of number of glyphs per side
static const size_t GLYPH_WIDTH_COUNT = 16;
static const size_t GLYPH_HEIGHT_COUNT = 16;

static void bind_texture(window_t* window, unsigned int texture) {
  if (window->last_bound_texture != texture) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    window->last_bound_texture = texture;
  }
}
static void load_glyph_font(glyph_shader_program_t* shader, const float* size) {
  if (glIsTexture(shader->font_texture)) {
    glDeleteTextures(1, &shader->font_texture);
  }

  int components;
  unsigned char* bitmap = stbi_load_from_memory(
      embed_tileset, sizeof(embed_tileset) / sizeof(unsigned char),
      &shader->texture_w, &shader->texture_h, &components, 0);

  if (shader->texture_w < 0 || shader->texture_h < 0) {
    log_error("Failed to read glyph texture");
  } else {
    // set glyph size on window so that it's accessible to the script
    // environment
    shader->window->glyph.width = (int)floorf(
        (float)shader->texture_w / GLYPH_WIDTH_COUNT * shader->scale);
    shader->window->glyph.height = (int)floorf(
        (float)shader->texture_h / GLYPH_HEIGHT_COUNT * shader->scale);

    // create font texture from bitmap
    glGenTextures(1, &shader->font_texture);
    bind_texture(shader->window, shader->font_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, shader->texture_w, shader->texture_h,
                 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);

    // set font texture filtering style
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }

  // cleanup bitmap data
  stbi_image_free(bitmap);
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

/* --------------------------- */
/* Public interface definition */
/* --------------------------- */

glyph_shader_program_t procy_create_glyph_shader(window_t* window) {
  glyph_shader_program_t glyph_shader;

  glyph_shader.window = window;
  glyph_shader.font_texture = 0;
  glyph_shader.texture_w = -1;
  glyph_shader.texture_h = -1;
  glyph_shader.scale = window->text_scale;

  shader_program_t* prog = &glyph_shader.program;
  if ((prog->valid =
           procy_compile_vert_shader((char*)embed_glyph_vert, &prog->vertex) &&
           procy_compile_frag_shader((char*)embed_glyph_frag,
                                     &prog->fragment))) {
    // load font texture and codepoints
    load_glyph_font(&glyph_shader, NULL);

    // create vertex array
    glGenVertexArrays(1, &prog->vao);
    glBindVertexArray(prog->vao);

    // create vertex buffers
    prog->vbo_count = 2;
    prog->vbo = malloc(sizeof(GLuint) * prog->vbo_count);
    glGenBuffers(prog->vbo_count, prog->vbo);

    prog->valid &=
        procy_link_shader_program(prog->vertex, prog->fragment, &prog->program);

    if (prog->valid) {
      glyph_shader.u_ortho = glGetUniformLocation(prog->program, "u_Ortho");
      glyph_shader.u_sampler =
          glGetUniformLocation(prog->program, "u_GlyphTexture");
    }
  }

  return glyph_shader;
}

void procy_draw_glyph_shader(glyph_shader_program_t* shader, window_t* window,
                             draw_op_t* ops, size_t n) {
  size_t glyph_count = count_glyphs_in_ops_buffer(ops, n);
  if (glyph_count == 0) {
    return;
  }

  glyph_vertex_t vertices[glyph_count * 4];
  GLushort indices[glyph_count * 6];

  float scale = shader->scale;
  float glyph_w = (float)shader->texture_w / GLYPH_WIDTH_COUNT;
  float glyph_h = (float)shader->texture_h / GLYPH_HEIGHT_COUNT;
  float glyph_tw = glyph_w / (float)shader->texture_w;
  float glyph_th = glyph_h / (float)shader->texture_h;

  size_t glyph_index = 0;
  for (size_t i = 0; i < n; ++i) {
    draw_op_t* op = &ops[i];
    if (op->type != DRAW_OP_TEXT) {
      continue;
    }

    size_t vert_ix = glyph_index * 4;
    size_t index_ix = glyph_index * 6;
    unsigned char c = op->data.text.character;

    // screen coordinates
    float x = (float)op->x;
    float y = (float)op->y;

    // texture coordinates
    float tx =
        (float)(c % GLYPH_WIDTH_COUNT) * glyph_w / (float)shader->texture_w;
    float ty = floorf((float)c / GLYPH_HEIGHT_COUNT) * glyph_h /
               (float)shader->texture_h;

    // vertex attributes
    glyph_vertex_t top_left = {x, y, tx, ty, op->forecolor, op->backcolor};
    glyph_vertex_t top_right = {x + glyph_w * scale, y,
                                tx + glyph_tw,       ty,
                                op->forecolor,       op->backcolor};
    glyph_vertex_t bottom_left = {x,
                                  y + glyph_h * scale,
                                  tx,
                                  ty + glyph_th,
                                  op->forecolor,
                                  op->backcolor};
    glyph_vertex_t bottom_right = {x + glyph_w * scale, y + glyph_h * scale,
                                   tx + glyph_tw,       ty + glyph_th,
                                   op->forecolor,       op->backcolor};

    vertices[vert_ix] = top_left;
    vertices[vert_ix + 1] = top_right;
    vertices[vert_ix + 2] = bottom_left;
    vertices[vert_ix + 3] = bottom_right;

    // indices
    //
    // first triangle
    indices[index_ix] = vert_ix;
    indices[index_ix + 1] = vert_ix + 1;
    indices[index_ix + 2] = vert_ix + 2;

    // second triangle
    indices[index_ix + 3] = vert_ix + 1;
    indices[index_ix + 4] = vert_ix + 3;
    indices[index_ix + 5] = vert_ix + 2;

    ++glyph_index;
  }

  shader_program_t* prog = &shader->program;
  glUseProgram(prog->program);

  bind_texture(window, shader->font_texture);

  // set font texture attribute
  glUniform1i(shader->u_sampler, GL_TEXTURE0);

  // set orthographic projection matrix
  glUniformMatrix4fv(shader->u_ortho, 1, GL_FALSE, &window->ortho[0][0]);

  // copy vertex data to video memory
  glBindBuffer(GL_ARRAY_BUFFER, prog->vbo[VBO_GLYPH_POSITION]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // associate the vertex data with the correct shader attribute
  glEnableVertexAttribArray(ATTR_GLYPH_POSITION);
  glVertexAttribPointer(ATTR_GLYPH_POSITION, 2, GL_FLOAT, GL_FALSE,
                        sizeof(glyph_vertex_t), 0);

  glEnableVertexAttribArray(ATTR_GLYPH_TEXCOORDS);
  glVertexAttribPointer(ATTR_GLYPH_TEXCOORDS, 2, GL_FLOAT, GL_FALSE,
                        sizeof(glyph_vertex_t), (void*)(2 * sizeof(float)));

  glEnableVertexAttribArray(ATTR_GLYPH_FORECOLOR);
  glVertexAttribPointer(ATTR_GLYPH_FORECOLOR, 3, GL_FLOAT, GL_FALSE,
                        sizeof(glyph_vertex_t), (void*)(4 * sizeof(float)));

  glEnableVertexAttribArray(ATTR_GLYPH_BACKCOLOR);
  glVertexAttribPointer(ATTR_GLYPH_BACKCOLOR, 3, GL_FLOAT, GL_FALSE,
                        sizeof(glyph_vertex_t), (void*)(7 * sizeof(float)));

  // copy indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prog->vbo[VBO_GLYPH_INDICES]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  glPolygonMode(GL_FRONT, GL_FILL);

  glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]),
                 GL_UNSIGNED_SHORT, 0);

  glDisableVertexAttribArray(ATTR_GLYPH_POSITION);
  glDisableVertexAttribArray(ATTR_GLYPH_TEXCOORDS);
  glDisableVertexAttribArray(ATTR_GLYPH_FORECOLOR);
  glDisableVertexAttribArray(ATTR_GLYPH_BACKCOLOR);
  glUseProgram(0);
}

void procy_destroy_glyph_shader_program(glyph_shader_program_t* shader) {
  if (shader != NULL) {
    procy_destroy_shader_program(&shader->program);

    // delete font texture
    if (glIsTexture(shader->font_texture)) {
      glDeleteTextures(1, &shader->font_texture);
    }
  }
}
