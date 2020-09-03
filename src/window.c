#include "window.h"

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <log.h>
#include <string.h>

#include "shader.h"
#include "shader/glyph.h"
#include "shader/rect.h"
#include "shader/line.h"
#include "drawing.h"
#include "keys.h"
#include "state.h"

typedef procy_window_t window_t;
typedef procy_window_bounds_t window_bounds_t;
typedef procy_draw_op_t draw_op_t;
typedef procy_draw_op_buffer_t draw_op_buffer_t;
typedef procy_glyph_shader_program_t glyph_shader_program_t;
typedef procy_rect_shader_program_t rect_shader_program_t;
typedef procy_line_shader_program_t line_shader_program_t;
typedef procy_key_info_t key_info_t;
typedef procy_color_t color_t;
typedef procy_state_t state_t;

static const size_t INITIAL_DRAW_OPS_BUFFER_SIZE = 1024;

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

static void window_resized(GLFWwindow* w, int width, int height) {
  log_debug("Window resized to %dx%d", width, height);

  glViewport(0, 0, width, height);

  window_t* window = (window_t*)glfwGetWindowUserPointer(w);
  window->bounds.width = width;
  window->bounds.height = height;

  set_ortho_projection(window);

  if (window->state->on_resize != NULL) {
    window->state->on_resize(window->state, width, height);
  }
}

static void set_window_callbacks(GLFWwindow* w) {
  glfwSetFramebufferSizeCallback(w, window_resized);
}

static void set_default_window_bounds(window_t* w, int width, int height) {
  w->bounds.width = width;
  w->bounds.height = height;
}

static void set_default_glyph_bounds(window_t* w) {
  w->glyph.width = -1;
  w->glyph.height = -1;
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

  log_debug("Initial draw ops buffer size: %zu", buffer_size);
}

static void expand_draw_ops_buffer(draw_op_buffer_t* draw_ops) {
  draw_ops->capacity *= 2;
  size_t buffer_size = draw_ops->capacity * sizeof(draw_op_t);
  draw_ops->buffer = realloc(draw_ops->buffer, buffer_size);

  log_trace("Expanded string draw ops buffer size to %zu bytes", buffer_size);
}

static void reset_draw_ops_buffer(draw_op_buffer_t* draw_ops) {
  // start writing draw ops at the start of the buffer
  draw_ops->length = 0;
}

static void handle_key_entered(GLFWwindow* w, int key, int scancode, int action,
                               int mods) {
  window_t* window = glfwGetWindowUserPointer(w);
  bool shift = (mods & GLFW_MOD_SHIFT) == GLFW_MOD_SHIFT;
  bool ctrl = (mods & GLFW_MOD_CONTROL) == GLFW_MOD_CONTROL;
  bool alt = (mods & GLFW_MOD_ALT) == GLFW_MOD_ALT;
  if (action == GLFW_PRESS && window->state->on_key_pressed != NULL) {
    window->state->on_key_pressed(window->state, window->key_table[key], shift,
                                  ctrl, alt);
  } else if (action == GLFW_RELEASE && window->state->on_key_released != NULL) {
    window->state->on_key_released(window->state, window->key_table[key], shift,
                                   ctrl, alt);
  }
}

static void handle_char_entered(GLFWwindow* w, unsigned int codepoint) {
  window_t* window = glfwGetWindowUserPointer(w);
  if (window->state->on_char_entered != NULL) {
    window->state->on_char_entered(window->state, codepoint);
  }
}

static void set_event_callbacks(window_t* w) {
  glfwSetKeyCallback(w->glfw_win, handle_key_entered);
  glfwSetCharCallback(w->glfw_win, handle_char_entered);
}

static void init_key_table(window_t* w) {
  // map key values to objects
  size_t keys_count = 0;
  key_info_t* keys = NULL;
  procy_get_keys(&keys, &keys_count);
  w->key_table = malloc(keys[keys_count - 1].value * sizeof(key_info_t));
  for (size_t i = 0; i < keys_count - 1; i++) {
    w->key_table[keys[i].value] = keys[i];
  }
  free(keys);
}

/* --------------------------- */
/* Public interface definition */
/* --------------------------- */

window_t* procy_create_window(int width, int height, const char* title,
                              float text_scale, state_t* state) {
  glfwSetErrorCallback(glfw_error_callback);
  window_t* window = malloc(sizeof(window_t));

  set_default_window_bounds(window, width, height);
  set_default_glyph_bounds(window);
  if (set_gl_window_pointer(window)) {
    set_ortho_projection(window);
    set_event_callbacks(window);
    init_draw_ops_buffer(&window->draw_ops);
    init_key_table(window);

    window->quitting = false;
    window->high_fps = false;
    window->last_bound_texture = UINT32_MAX;
    window->text_scale = text_scale;
    window->state = state;
  } else {
    free(window);
    return NULL;
  }

  return window;
}

void procy_destroy_window(window_t* window) {
  if (window == NULL) {
    return;
  }

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

void procy_append_draw_op(window_t* window, draw_op_t* draw_op) {
  draw_op_buffer_t* draw_ops = &window->draw_ops;
  draw_ops->length++;
  if (draw_ops->length >= draw_ops->capacity) {
    expand_draw_ops_buffer(draw_ops);
  }
  draw_ops->buffer[draw_ops->length - 1] = *draw_op;
}

void procy_begin_loop(window_t* window) {
  // set up shaders
  glyph_shader_program_t glyph_shader = procy_create_glyph_shader(window);
  rect_shader_program_t rect_shader = procy_create_rect_shader();
  line_shader_program_t line_shader = procy_create_line_shader();

  // this can be overridden later, but black is a good default
  glClearColor(0.0F, 0.0F, 0.0F, 1.0F);

  state_t* state = window->state;
  if (state->on_load != NULL) {
    state->on_load(state);
  }

  // initialize the running-timer to 0.0 seconds so that we can later judge how
  // long the main loop has been running for (for no other reason than as a
  // convenient benchmarking tool)
  glfwSetTime(0.0);

  double last_frame_time = glfwGetTime();
  GLFWwindow* w = (GLFWwindow*)window->glfw_win;
  while (!glfwWindowShouldClose(w) && !window->quitting) {
    double current_time = glfwGetTime();
    double frame_duration = current_time - last_frame_time;
    last_frame_time = current_time;

    if (window->high_fps) {
      glfwPollEvents();
    } else {
      glfwWaitEventsTimeout(2.0F);
    }

    glClear(GL_COLOR_BUFFER_BIT);

    if (state->on_draw != NULL) {
      state->on_draw(state, frame_duration);
    }

    if (window->draw_ops.length > 0) {
      if (glyph_shader.program.valid) {
        procy_draw_glyph_shader(&glyph_shader, window);
      }

      if (rect_shader.program.valid) {
        procy_draw_rect_shader(&rect_shader, window);
      }

      if (line_shader.program.valid) {
        procy_draw_line_shader(&line_shader, window);
      }

      reset_draw_ops_buffer(&window->draw_ops);
    }

    glfwSwapBuffers(w);
  }

  if (state->on_unload != NULL) {
    state->on_unload(state);
  }

  procy_destroy_glyph_shader(&glyph_shader);
  procy_destroy_rect_shader(&rect_shader);
  procy_destroy_line_shader(&line_shader);
}

void procy_set_clear_color(color_t c) { glClearColor(c.r, c.g, c.b, 1.0F); }

void procy_close_window(procy_window_t* window) { window->quitting = true; }
