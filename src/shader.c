#include "shader.h"

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <log.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#ifdef EXPORT_GLYPH_BITMAP
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#endif

#include "gen/glyph_frag.h"
#include "gen/glyph_vert.h"
#include "gen/vga_ttf.h"
#include "window.h"
#include "drawing.h"

#pragma pack(0)
typedef struct glyph_vertex_t {
  float x, y, u, v;
} glyph_vertex_t;
#pragma pack(1)

static const size_t VBO_GLYPH_POSITION = 0;
static const size_t VBO_GLYPH_INDICES = 1;
static const size_t ATTR_GLYPH_POSITION = 0;
static const size_t ATTR_GLYPH_TEXCOORDS = 1;

static const float FONT_SIZE = 16.0F;
static const int FONT_TEXTURE_WIDTH = 256;
static const int FONT_TEXTURE_HEIGHT = 128;
static const float FONT_PADDING = 8.0F;

// the font we're using supports full "extended" ASCII (0 thru 256)
static const int FONT_CODEPOINTS = 256;

static void bind_texture(window_t* window, unsigned int texture) {
  if (!is_window_texture_bound(window, texture)) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    set_window_bound_texture(window, texture);
  }
}

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

static void load_glyph_font(glyph_shader_program_t* shader, const float* size) {
  // load font codepoint geometry and texture coordinates and bitmap
  unsigned char* bitmap_buffer =
      malloc(FONT_TEXTURE_WIDTH * FONT_TEXTURE_HEIGHT * sizeof(unsigned char));

  if (shader->codepoints != NULL) {
    free(shader->codepoints);
  }

  if (glIsTexture(shader->font_texture)) {
    glDeleteTextures(1, &shader->font_texture);
  }

  shader->codepoints = malloc(sizeof(stbtt_bakedchar) * FONT_CODEPOINTS);

  stbtt_BakeFontBitmap(embed_vga_ttf, 0, (size == NULL ? FONT_SIZE : *size),
                       bitmap_buffer, FONT_TEXTURE_WIDTH, FONT_TEXTURE_HEIGHT,
                       0, FONT_CODEPOINTS,
                       (stbtt_bakedchar*)shader->codepoints);

  // create font texture from bitmap
  glGenTextures(1, &shader->font_texture);
  bind_texture(shader->window, shader->font_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, FONT_TEXTURE_WIDTH,
               FONT_TEXTURE_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap_buffer);

#ifdef EXPORT_GLYPH_BITMAP
  stbi_write_bmp("output.bmp", FONT_TEXTURE_WIDTH, FONT_TEXTURE_HEIGHT, 1,
                 bitmap_buffer);
#endif

  // cleanup bitmap data
  free(bitmap_buffer);

  // set font texture filtering style
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

static size_t count_glyphs_in_ops_buffer(draw_op_t* ops, size_t n) {
  // get total number of characters
  size_t glyph_count = 0;
  for (size_t i = 0; i < n; ++i) {
    if (ops[i].type != DRAW_OP_TEXT) {
      continue;
    }

    for (size_t j = 0; ops[i].data.text.contents[j] != '\0'; j++) {
      ++glyph_count;
    }
  }

  return glyph_count;
}

glyph_shader_program_t create_glyph_shader(window_t* window) {
  glyph_shader_program_t glyph_shader;
  glyph_shader.codepoints = NULL;
  glyph_shader.window = window;
  glyph_shader.font_texture = 0;

  shader_program_t* prog = &glyph_shader.program;
  if ((prog->valid =
           compile_vert_shader((char*)embed_glyph_vert, &prog->vertex) &&
           compile_frag_shader((char*)embed_glyph_frag, &prog->fragment))) {
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
        link_shader_program(prog->vertex, prog->fragment, &prog->program);

    if (prog->valid) {
      glyph_shader.u_ortho = glGetUniformLocation(prog->program, "u_Ortho");
      glyph_shader.u_sampler =
          glGetUniformLocation(prog->program, "u_GlyphTexture");
    }
  }

  return glyph_shader;
}

void draw_glyph_shader(glyph_shader_program_t* shader, window_t* window,
                       draw_op_t* ops, size_t n) {
  size_t glyph_count = count_glyphs_in_ops_buffer(ops, n);
  if (glyph_count == 0) {
    return;
  }

  glyph_vertex_t vertices[glyph_count * 4];
  GLushort indices[glyph_count * 6];

  size_t glyph_index = 0;
  for (size_t i = 0; i < n; ++i) {
    // iterate over each draw operation...
    if (ops[i].type != DRAW_OP_TEXT) {
      continue;
    }

    float x_offset = 0;
    draw_op_t* op = &ops[i];
    for (size_t j = 0; op->data.text.contents[j] != '\0'; j++) {
      // iterate over each character in the draw op's buffer...

      size_t vert_ix = glyph_index * 4;
      size_t index_ix = glyph_index * 6;
      float x = (float)op->x + x_offset;
      float y = (float)op->y;

      stbtt_aligned_quad quad;  // current glyph bounds
      stbtt_GetBakedQuad((const stbtt_bakedchar*)shader->codepoints,
                         FONT_TEXTURE_WIDTH, FONT_TEXTURE_HEIGHT,
                         op->data.text.contents[j], &x, &y, &quad, 1);

      // increment x-offset by the quad's width
      x_offset += FONT_PADDING;

      // vertex positions
      //
      // top left
      vertices[vert_ix].x = quad.x0;
      vertices[vert_ix].y = quad.y0;
      vertices[vert_ix].u = quad.s0;
      vertices[vert_ix].v = quad.t0;

      // top right
      vertices[vert_ix + 1].x = quad.x1;
      vertices[vert_ix + 1].y = quad.y0;
      vertices[vert_ix + 1].u = quad.s1;
      vertices[vert_ix + 1].v = quad.t0;

      // bottom left
      vertices[vert_ix + 2].x = quad.x0;
      vertices[vert_ix + 2].y = quad.y1;
      vertices[vert_ix + 2].u = quad.s0;
      vertices[vert_ix + 2].v = quad.t1;

      // bottom right
      vertices[vert_ix + 3].x = quad.x1;
      vertices[vert_ix + 3].y = quad.y1;
      vertices[vert_ix + 3].u = quad.s1;
      vertices[vert_ix + 3].v = quad.t1;

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

  // copy indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prog->vbo[VBO_GLYPH_INDICES]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  glPolygonMode(GL_FRONT, GL_FILL);

  glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]),
                 GL_UNSIGNED_SHORT, 0);

  glDisableVertexAttribArray(ATTR_GLYPH_POSITION);
  glDisableVertexAttribArray(ATTR_GLYPH_TEXCOORDS);
  glUseProgram(0);
}

static void destroy_shader_program(shader_program_t* shader) {
  // detach shaders from program and delete program
  if (glIsProgram(shader->program) == GL_TRUE) {
    if (glIsShader(shader->fragment) == GL_TRUE) {
      glDetachShader(shader->program, shader->fragment);
    }
    if (glIsShader(shader->vertex) == GL_TRUE) {
      glDetachShader(shader->program, shader->vertex);
    }
    glDeleteProgram(shader->program);
  }

  // delete shaders
  if (glIsShader(shader->fragment) == GL_TRUE) {
    glDeleteShader(shader->fragment);
  }
  if (glIsShader(shader->vertex) == GL_TRUE) {
    glDeleteShader(shader->vertex);
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

  shader->valid = false;
}

void destroy_glyph_shader_program(glyph_shader_program_t* shader) {
  if (shader != NULL) {
    destroy_shader_program(&shader->program);

    // free codepoint data
    if (shader->codepoints != NULL) {
      free(shader->codepoints);
    }

    // delete font texture
    if (glIsTexture(shader->font_texture)) {
      glDeleteTextures(1, &shader->font_texture);
    }
  }
}
