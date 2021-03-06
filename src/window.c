#include "window.h"

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <log.h>
#include <string.h>

#include "shader.h"
#include "shader/error.h"
#include "shader/glyph.h"
#include "shader/rect.h"
#include "shader/line.h"
#include "drawing.h"
#include "keys.h"
#include "state.h"

typedef procy_window_t window_t;
typedef procy_shaders_t shaders_t;
typedef procy_draw_op_t draw_op_t;
typedef procy_draw_op_buffer_t draw_op_buffer_t;
typedef procy_glyph_shader_program_t glyph_shader_program_t;
typedef procy_rect_shader_program_t rect_shader_program_t;
typedef procy_line_shader_program_t line_shader_program_t;
typedef procy_key_info_t key_info_t;
typedef procy_color_t color_t;
typedef procy_state_t state_t;

static const size_t INITIAL_DRAW_OPS_BUFFER_SIZE = 1024;

static void glfw_error_callback(int code, const char *msg) {
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

static int setup_gl_context(GLFWwindow *w) {
  glfwMakeContextCurrent(w);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    log_error("Failed to initialize OpenGL");
    return false;
  }
  return true;
}

static void set_ortho_projection(window_t *window, int width, int height) {
  // zero-out matrix
  memset(&window->ortho[0][0], 0, 4 * sizeof(float));
  memset(&window->ortho[1][0], 0, 4 * sizeof(float));
  memset(&window->ortho[2][0], 0, 4 * sizeof(float));
  memset(&window->ortho[3][0], 0, 4 * sizeof(float));

  // build orthographic projection based on window dimensions
  window->ortho[0][0] = 2.0F / (float)width;
  window->ortho[0][3] = -1.0F;
  window->ortho[1][1] = 2.0F / -(float)height;
  window->ortho[1][3] = 1.0F;
  window->ortho[2][2] = -2.0F;
  window->ortho[2][3] = -1.0F;
  window->ortho[3][3] = 1.0F;
}

static void window_resized(GLFWwindow *w, int width, int height) {
  log_debug("Window resized to %dx%d", width, height);

  GL_CHECK(glViewport(0, 0, width, height));

  window_t *window = (window_t *)glfwGetWindowUserPointer(w);

  set_ortho_projection(window, width, height);
  state_t *state = window->state;
  if (state->on_resize != NULL) {
    state->on_resize(state, width, height);
  }
}

static void set_window_callbacks(GLFWwindow *w) {
  glfwSetFramebufferSizeCallback(w, window_resized);
}

static void destroy_shaders(shaders_t *shaders) {
  procy_destroy_glyph_shader(shaders->glyph);
  procy_destroy_rect_shader(shaders->rect);
  procy_destroy_line_shader(shaders->line);
}

/*
 * Returns false upon failing to initialize a GLFW window
 */
static bool set_gl_window_pointer(window_t *w, int width, int height,
                                  const char *title) {
  if (!glfwInit()) {
    return false;
  }

  set_gl_hints();

  w->glfw_win = glfwCreateWindow(width, height, title, NULL, NULL);
  if (w->glfw_win == NULL || !setup_gl_context(w->glfw_win)) {
    glfwTerminate();
    return false;
  }

  glfwSetWindowUserPointer(w->glfw_win, w);
  set_window_callbacks(w->glfw_win);

  glViewport(0, 0, width, height);

  return true;
}

static void init_draw_ops_buffer(draw_op_buffer_t *draw_ops) {
  draw_ops->length = 0;
  draw_ops->capacity = INITIAL_DRAW_OPS_BUFFER_SIZE;

  size_t buffer_size = sizeof(draw_op_t) * draw_ops->capacity;
  draw_ops->buffer = malloc(buffer_size);

  log_debug("Initial draw ops buffer size: %zu", buffer_size);
}

static void expand_draw_ops_buffer(draw_op_buffer_t *draw_ops) {
  draw_ops->capacity *= 2;
  size_t buffer_size = draw_ops->capacity * sizeof(draw_op_t);
  draw_op_t *resized = realloc(draw_ops->buffer, buffer_size);
  if (resized == NULL) {
    free(draw_ops->buffer);
    draw_ops->buffer = malloc(INITIAL_DRAW_OPS_BUFFER_SIZE);
    draw_ops->capacity = 1;
    draw_ops->length = 0;
  } else {
    draw_ops->buffer = resized;
  }

  log_trace("Expanded draw ops buffer size to %zu bytes", buffer_size);
}

static void reset_draw_ops_buffer(draw_op_buffer_t *draw_ops) {
  // start writing draw ops at the start of the buffer
  draw_ops->length = 0;
}

static void handle_key_entered(GLFWwindow *w, int key, int scancode, int action,
                               int mods) {
  window_t *window = glfwGetWindowUserPointer(w);
  bool shift = (mods & GLFW_MOD_SHIFT) == GLFW_MOD_SHIFT;
  bool ctrl = (mods & GLFW_MOD_CONTROL) == GLFW_MOD_CONTROL;
  bool alt = (mods & GLFW_MOD_ALT) == GLFW_MOD_ALT;
  state_t *state = window->state;
  if ((action == GLFW_PRESS || action == GLFW_REPEAT) &&
      state->on_key_pressed != NULL) {
    state->on_key_pressed(state, window->key_table[key], shift, ctrl, alt);
  } else if (action == GLFW_RELEASE && state->on_key_released != NULL) {
    state->on_key_released(state, window->key_table[key], shift, ctrl, alt);
  }
}

static void handle_char_entered(GLFWwindow *w, unsigned int codepoint) {
  window_t *window = glfwGetWindowUserPointer(w);
  state_t *state = window->state;
  if (state->on_char_entered != NULL) {
    state->on_char_entered(state, codepoint);
  }
}

static void set_event_callbacks(window_t *w) {
  glfwSetKeyCallback(w->glfw_win, handle_key_entered);
  glfwSetCharCallback(w->glfw_win, handle_char_entered);
}

static void init_key_table(window_t *w) {
  // map key values to objects
  size_t keys_count = 0;
  key_info_t *keys = NULL;
  procy_get_keys(&keys, &keys_count);
  w->key_table = malloc(keys[keys_count - 1].value * sizeof(key_info_t));
  for (size_t i = 0; i < keys_count - 1; i++) {
    w->key_table[keys[i].value] = keys[i];
  }
  free(keys);
}

static void init_shaders(window_t *window, float text_scale) {
  window->shaders.glyph = procy_create_glyph_shader(text_scale);
  window->shaders.rect = procy_create_rect_shader();
  window->shaders.line = procy_create_line_shader();
}

static void log_opengl_info() {
  log_debug("Vendor: %s", glGetString(GL_VENDOR));
  log_debug("Renderer: %s", glGetString(GL_RENDERER));
  log_debug("OpenGL Version: %s", glGetString(GL_VERSION));
}

/* --------------------------- */
/* Public interface definition */
/* --------------------------- */

window_t *procy_create_window(int width, int height, const char *title,
                              float text_scale, state_t *state) {
  glfwSetErrorCallback(glfw_error_callback);
  window_t *window = malloc(sizeof(window_t));

  if (window != NULL && set_gl_window_pointer(window, width, height, title)) {
    log_opengl_info();
    init_shaders(window, text_scale);
    set_ortho_projection(window, width, height);
    set_event_callbacks(window);
    init_draw_ops_buffer(&window->draw_ops);
    init_key_table(window);

    window->quitting = false;
    window->high_fps = false;
    window->state = state;
  } else {
    free(window);
    return NULL;
  }

  return window;
}

void procy_destroy_window(window_t *window) {
  if (window == NULL) {
    return;
  }

  destroy_shaders(&window->shaders);

  if (window->glfw_win != NULL) {
    glfwDestroyWindow(window->glfw_win);
  }

  if (window->draw_ops.buffer != NULL) {
    free(window->draw_ops.buffer);
  }

  if (window->key_table != NULL) {
    free(window->key_table);
  }

  glfwTerminate();
  free(window);
}

void procy_append_draw_op(window_t *window, draw_op_t *draw_op) {
  draw_op_buffer_t *buffer = &window->draw_ops;
  buffer->length++;
  if (buffer->length >= buffer->capacity) {
    expand_draw_ops_buffer(buffer);
  }
  buffer->buffer[buffer->length - 1] = *draw_op;
}

void procy_append_draw_ops(procy_window_t *window, draw_op_t *draw_ops,
                           size_t n) {
  // TODO: test this
  draw_op_buffer_t *buffer = &window->draw_ops;
  size_t dest_offset = buffer->length - 1;
  buffer->length += n;
  if (buffer->length >= buffer->capacity) {
    expand_draw_ops_buffer(buffer);
  }
  memcpy(&buffer->buffer[dest_offset], draw_ops, n * sizeof(draw_op_t));
}

void procy_get_window_size(window_t *window, int *width, int *height) {
  glfwGetWindowSize(window->glfw_win, width, height);
}

void procy_get_glyph_size(procy_window_t *window, int *width, int *height) {
  procy_get_glyph_bounds(window->shaders.glyph, width, height);
}

void procy_set_glyph_scale(procy_window_t *window, float scale) {
  window->shaders.glyph->glyph_scale = scale;
}

void procy_begin_loop(window_t *window) {
  // this can be overridden later, but black is a good default
  GL_CHECK(glClearColor(0.0F, 0.0F, 0.0F, 1.0F));

  state_t *state = window->state;
  if (state->on_load != NULL) {
    state->on_load(state);
  }

  // initialize the running-timer to 0.0 seconds so that we can later judge how
  // long the main loop has been running for (for no other reason than as a
  // convenient benchmarking tool)
  glfwSetTime(0.0);

  double last_frame_time = glfwGetTime();
  GLFWwindow *w = (GLFWwindow *)window->glfw_win;
  procy_glyph_shader_program_t *glyph_shader = window->shaders.glyph;
  procy_rect_shader_program_t *rect_shader = window->shaders.rect;
  procy_line_shader_program_t *line_shader = window->shaders.line;
  while (!glfwWindowShouldClose(w) && !window->quitting) {
    double current_time = glfwGetTime();
    double frame_duration = current_time - last_frame_time;
    last_frame_time = current_time;

    if (window->high_fps) {
      glfwPollEvents();
    } else {
      glfwWaitEventsTimeout(1.0);
    }

    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));

    if (state->on_draw != NULL) {
      state->on_draw(state, frame_duration);
    }

    if (window->draw_ops.length > 0) {
      if (rect_shader->program.valid) {
        procy_draw_rect_shader(rect_shader, window);
      }

      if (line_shader->program.valid) {
        procy_draw_line_shader(line_shader, window);
      }

      if (glyph_shader->program.valid) {
        procy_draw_glyph_shader(glyph_shader, window);
      }

      reset_draw_ops_buffer(&window->draw_ops);
    }

    glfwSwapBuffers(w);
  }

  if (state->on_unload != NULL) {
    state->on_unload(state);
  }
}

void procy_set_clear_color(color_t c) { glClearColor(c.r, c.g, c.b, 1.0F); }

void procy_close_window(procy_window_t *window) { window->quitting = true; }

void procy_set_high_fps_mode(procy_window_t *window, bool high_fps) {
  window->high_fps = high_fps;
}
