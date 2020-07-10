#include "window.h"

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <log.h>
#include <stdlib.h>
#include <string.h>

#include "shader.h"
#include "config.h"
#include "drawing.h"

static const size_t INITIAL_DRAW_OPS_BUFFER_SIZE = 32;

static void glfw_error_callback(int code, const char* msg) {
  log_error("GLFW error %d: %s", code, msg);
}

static void set_gl_hints() {
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}

static int setup_gl_context(GLFWwindow* w) {
  glfwMakeContextCurrent(w);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    log_error("Failed to initialize OpenGL");
    return false;
  }
  return true;
}

static void set_ortho_projection(window_t* window) {
  int width = window->bounds.width;
  int height = window->bounds.height;

  for (int i = 0; i < 4; ++i) {
    memset(&window->ortho[i][0], 0, 4 * sizeof(float));
  }

  window->ortho[0][0] = 2.0F / (float)width;
  window->ortho[0][3] = -1.0F;
  window->ortho[1][1] = 2.0F / -(float)height;
  window->ortho[1][3] = 1.0F;
  window->ortho[2][2] = -2.0F;
  window->ortho[2][3] = -1.0F;
  window->ortho[3][3] = 1.0F;
}

static void window_resized(GLFWwindow* w, int width, int height) {
  log_debug("Window resized to %dx%d", width, height);

  glViewport(0, 0, width, height);

  window_t* window = (window_t*)glfwGetWindowUserPointer(w);
  window->bounds.width = width;
  window->bounds.height = height;

  set_ortho_projection(window);

  set_window_state_dirty(window);
}

static void set_window_callbacks(GLFWwindow* w) {
  glfwSetFramebufferSizeCallback(w, window_resized);
}

static void set_default_window_bounds(window_t* w, config_t* cfg) {
  w->bounds.width = cfg->window_w;
  w->bounds.height = cfg->window_h;
}

static void set_default_tile_bounds(window_t* w, config_t* cfg) {
  w->tile_bounds.width = cfg->tile_w;
  w->tile_bounds.height = cfg->tile_h;
}

/*
 * Returns false upon failing to initialize a GLFW window
 */
static bool set_gl_window_pointer(window_t* w) {
  if (!glfwInit()) {
    return false;
  }

  set_gl_hints();

  window_bounds_t bounds = w->bounds;
  w->glfw_win =
      glfwCreateWindow(bounds.width, bounds.height, "Surulia", NULL, NULL);
  if (w->glfw_win == NULL || !setup_gl_context(w->glfw_win)) {
    glfwTerminate();
    return false;
  }

  glfwSetWindowUserPointer(w->glfw_win, w);
  set_window_callbacks(w->glfw_win);

  glViewport(0, 0, bounds.width, bounds.height);

  return true;
}

static void init_draw_ops_buffer(draw_op_buffer_t* draw_ops) {
  draw_ops->length = 0;
  draw_ops->capacity = INITIAL_DRAW_OPS_BUFFER_SIZE;

  size_t buffer_size = sizeof(draw_op_t) * draw_ops->capacity;
  draw_ops->buffer = malloc(buffer_size);

  log_debug("Initial string draw op buffer size: %zu", buffer_size);
}

static void expand_draw_ops_buffer(draw_op_buffer_t* draw_ops) {
  size_t buffer_size = draw_ops->capacity * sizeof(draw_op_t);
  draw_ops->capacity *= 2;
  draw_ops->buffer = realloc(draw_ops->buffer, buffer_size);

  log_trace("Expanded string draw operations buffer size to %zu bytes",
            buffer_size);
}

static void clear_draw_ops_buffer(draw_op_buffer_t* draw_ops) {
  log_trace("Clearing draw operations buffer...");

  draw_ops->length = 0;
  draw_ops->capacity = INITIAL_DRAW_OPS_BUFFER_SIZE;
  size_t string_ops_buffer_size = draw_ops->capacity * sizeof(draw_op_t);
  draw_ops->buffer = realloc(draw_ops->buffer, string_ops_buffer_size);

  log_trace("String draw operations buffer shrunk to %zu bytes",
            string_ops_buffer_size);
}

window_t* create_window(config_t* cfg) {
  glfwSetErrorCallback(glfw_error_callback);
  window_t* window = malloc(sizeof(window_t));

  set_default_window_bounds(window, cfg);
  set_gl_window_pointer(window);
  set_default_tile_bounds(window, cfg);
  set_window_state_dirty(window);
  set_ortho_projection(window);
  init_draw_ops_buffer(&window->draw_ops);

  window->script_state = NULL;
  window->glyph_shader = NULL;
  window->quitting = false;
  window->last_bound_texture = UINT32_MAX;

  return window;
}

void destroy_window(window_t* window) {
  if (window == NULL) {
    return;
  }

  if (window->glfw_win != NULL) {
    glfwDestroyWindow(window->glfw_win);
  }

  if (window->draw_ops.buffer != NULL) {
    free(window->draw_ops.buffer);
  }

  glfwTerminate();
  free(window);
}

void append_string_draw_op(window_t* window, int x, int y,
                           const char* contents) {
  draw_op_buffer_t* draw_ops = &window->draw_ops;
  if (draw_ops->length + 1 > draw_ops->capacity) {
    expand_draw_ops_buffer(draw_ops);
  }

  draw_ops->buffer[draw_ops->length++] = create_draw_op_string(x, y, contents);
  set_window_state_dirty(window);
}

void set_window_state_dirty(window_t* w) { w->state = WINDOW_STATE_DIRTY; }

void set_window_state_wait(window_t* w) { w->state = WINDOW_STATE_WAIT; }

void set_window_bound_texture(window_t* w, unsigned int tex) {
  w->last_bound_texture = tex;
}

bool is_window_texture_bound(window_t* w, unsigned int tex) {
  return w->last_bound_texture == tex;
}

void begin_loop(window_t* window) {
  glyph_shader_program_t glyph_shader = create_glyph_shader();

  // store a reference to the glyph shader so that script functions can modify
  // text rendering
  window->glyph_shader = &glyph_shader;

  GLFWwindow* w = (GLFWwindow*)window->glfw_win;
  while (!glfwWindowShouldClose(w) && !window->quitting) {
    glfwWaitEventsTimeout(0.5F);

    if (window->state == WINDOW_STATE_DIRTY) {
      glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
      glClear(GL_COLOR_BUFFER_BIT);

      if (window->draw_ops.length > 0 && glyph_shader.program.valid) {
        draw_glyph_shader(&glyph_shader, window, window->draw_ops.buffer,
                          window->draw_ops.length);
        clear_draw_ops_buffer(&window->draw_ops);
      }

      glfwSwapBuffers(w);
      set_window_state_wait(window);
    }
  }

  destroy_glyph_shader_program(&glyph_shader);
}
