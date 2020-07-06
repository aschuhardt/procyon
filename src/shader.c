#include "shader.h"

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <log.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include "gen/glyph_frag.h"
#include "gen/glyph_vert.h"
#include "gen/vga_ttf.h"
#include "window.h"

#pragma pack(0)
typedef struct glyph_vertex_t {
  float x, y, u, v;
} glyph_vertex_t;
#pragma pack(1)

static const size_t VBO_GLYPH_POSITION = 0;
static const size_t VBO_GLYPH_INDICES = 1;
static const size_t ATTR_GLYPH_POSITION = 0;
static const size_t ATTR_GLYPH_TEXCOORDS = 1;

static const float FONT_SIZE = 32.0F;
static const int FONT_TEXTURE_SIZE = 512;

// the font we're using supports full "extended" ASCII (0 thru 256)
static const int FONT_CODEPOINTS = 256;

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

glyph_shader_program_t create_glyph_shader() {
  glyph_shader_program_t glyph_shader;
  shader_program_t* prog = &glyph_shader.program;
  if ((prog->valid =
           compile_vert_shader((char*)embed_glyph_vert, &prog->vertex) &&
           compile_frag_shader((char*)embed_glyph_frag, &prog->fragment))) {
    // create vertex array
    glGenVertexArrays(1, &prog->vao);
    glBindVertexArray(prog->vao);

    // create vertex buffers
    prog->vbo_count = 2;
    prog->vbo = malloc(sizeof(GLuint) * prog->vbo_count);
    glGenBuffers(prog->vbo_count, prog->vbo);

    // load font codepoint geometry and texture coordinates and bitmap
    unsigned char* bitmap_buffer =
        malloc(FONT_TEXTURE_SIZE * FONT_TEXTURE_SIZE * sizeof(unsigned char));
    glyph_shader.codepoints = malloc(sizeof(stbtt_bakedchar) * FONT_CODEPOINTS);
    stbtt_BakeFontBitmap(embed_vga_ttf, 0, FONT_SIZE, bitmap_buffer,
                         FONT_TEXTURE_SIZE, FONT_TEXTURE_SIZE, 0,
                         FONT_CODEPOINTS,
                         (stbtt_bakedchar*)glyph_shader.codepoints);

    // create font texture from bitmap
    glGenTextures(1, &glyph_shader.font_texture);
    glBindTexture(GL_TEXTURE_2D, glyph_shader.font_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, FONT_TEXTURE_SIZE, FONT_TEXTURE_SIZE,
                 0, GL_RED, GL_UNSIGNED_BYTE, bitmap_buffer);

    // cleanup bitmap data
    free(bitmap_buffer);

    // set font texture filtering style
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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

void draw_glyphs(glyph_shader_program_t* glyph_shader, window_t* window,
                 glyph_t* data, size_t count) {
  glyph_vertex_t verts[count * 4];
  GLushort indices[count * 6];

  float tile_w = (float)window->tile_bounds.width;
  float tile_h = (float)window->tile_bounds.height;
  for (int i = 0; i < count; ++i) {
    float x = (float)data[i].x * tile_w;
    float y = (float)data[i].y * tile_h;

    stbtt_aligned_quad tex_quad;
    stbtt_GetBakedQuad(glyph_shader->codepoints, FONT_TEXTURE_SIZE,
                       FONT_TEXTURE_SIZE, data[i].data, &x, &y, &tex_quad, 1);

    size_t vert_ix = i * 4;

    // top left
    verts[vert_ix].x = x;
    verts[vert_ix].y = y;
    verts[vert_ix].u = tex_quad.s0;
    verts[vert_ix].v = tex_quad.t0;

    // top right
    verts[vert_ix + 1].x = x + tile_w;
    verts[vert_ix + 1].y = y;
    verts[vert_ix + 1].u = tex_quad.s1;
    verts[vert_ix + 1].v = tex_quad.t0;

    // bottom left
    verts[vert_ix + 2].x = x;
    verts[vert_ix + 2].y = y + tile_h;
    verts[vert_ix + 2].u = tex_quad.s0;
    verts[vert_ix + 2].v = tex_quad.t1;

    // bottom right
    verts[vert_ix + 3].x = x + tile_w;
    verts[vert_ix + 3].y = y + tile_h;
    verts[vert_ix + 3].u = tex_quad.s1;
    verts[vert_ix + 3].v = tex_quad.t1;

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

  shader_program_t* prog = &glyph_shader->program;

  glUseProgram(prog->program);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, glyph_shader->font_texture);

  // set font texture attribute
  glUniform1i(glyph_shader->u_sampler, GL_TEXTURE0);

  // set orthographic projection matrix
  glUniformMatrix4fv(glyph_shader->u_ortho, 1, GL_FALSE, &window->ortho[0][0]);

  // copy vertex data to video memory
  glBindBuffer(GL_ARRAY_BUFFER, prog->vbo[VBO_GLYPH_POSITION]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

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

static void destroy_shader_program(shader_program_t* prog) {
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
    free(prog->vbo);
  }

  // delete vertex array
  if (glIsVertexArray(prog->vao)) {
    glDeleteVertexArrays(1, &prog->vao);
  }

  prog->valid = false;
}

void destroy_glyph_shader_program(glyph_shader_program_t* program) {
  if (program != NULL) {
    destroy_shader_program(&program->program);

    // free codepoint data
    if (program->codepoints != NULL) {
      free(program->codepoints);
    }

    // delete font texture
    if (glIsTexture(program->font_texture)) {
      glDeleteTextures(1, &program->font_texture);
    }
  }
}
