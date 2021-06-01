#ifndef SHADER_SPRITE_H
#define SHADER_SPRITE_H

#include "shader.h"

typedef struct procy_sprite_shader_program_t {
  procy_shader_program_t program;
  void *vertex_batch_buffer, *index_batch_buffer;
  unsigned int u_ortho, u_sampler, texture;
  int texture_w, texture_h;
} procy_sprite_shader_program_t;

/*
 * Builds and compiles a shader program capable of rendering sprites based on a
 * texture derived from an image file stored in-memory
 *
 */
procy_sprite_shader_program_t *procy_create_sprite_shader_mem(
    unsigned char *contents, size_t length);

/*
 * Builds and compiles a shader program capable of rendering sprites based on a
 * texture derived from an on-disk image file
 */

procy_sprite_shader_program_t *procy_create_sprite_shader(const char *path);

/*
 * Builds and executes a draw call on the GPU, consisting of vertex data built
 * from all of the `sprite` type draw operations
 */
void procy_draw_sprite_shader(procy_sprite_shader_program_t *shader,
                              struct procy_window_t *window);

/*
 * Disposes of a sprite shader program and deletes its bound resources from
 * the OpenGL context
 */
void procy_destroy_sprite_shader(procy_sprite_shader_program_t *shader);

#endif
