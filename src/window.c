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

void set_state_dirty(window_t* w) { w->state = WINDOW_STATE_DIRTY; }

void set_state_wait(window_t* w) { w->state = WINDOW_STATE_WAIT; }

static void window_resized(GLFWwindow* w, int width, int height) {
  log_debug("Window resized to %dx%d", width, height);

  glViewport(0, 0, width, height);

  window_t* window = (window_t*)glfwGetWindowUserPointer(w);
  window->bounds.width = width;
  window->bounds.height = height;

  set_state_dirty(window);
}

static void set_window_callbacks(GLFWwindow* w) {
  glfwSetFramebufferSizeCallback(w, window_resized);
}

static void clear_glyph_buffer(window_t* window) {
  window->glyph_count = 0;
  memset(&window->glyph_buffer[0], 0, GLYPH_BUFFER_SIZE);
}

window_t* create_window(config_t* cfg) {
  glfwSetErrorCallback(glfw_error_callback);

  if (!glfwInit()) {
    return NULL;
  }

  set_gl_hints();

  GLFWwindow* glfw_win =
      glfwCreateWindow(cfg->window_w, cfg->window_h, "Surulia", NULL, NULL);
  if (glfw_win == NULL || !setup_gl_context(glfw_win)) {
    glfwTerminate();
    return NULL;
  }

  glViewport(0, 0, cfg->window_w, cfg->window_h);

  window_t* window = malloc(sizeof(window_t));

  window->glfw_win = glfw_win;
  window->script_state = NULL;
  window->state = WINDOW_STATE_DIRTY;
  window->quitting = false;
  window->tile_bounds.width = cfg->tile_w;
  window->tile_bounds.height = cfg->tile_h;
  window->bounds.width = cfg->window_w;
  window->bounds.height = cfg->window_h;

  clear_glyph_buffer(window);

  glfwSetWindowUserPointer(glfw_win, window);
  set_window_callbacks(glfw_win);

  return window;
}

void destroy_window(window_t* window) {
  if (window == NULL) {
    return;
  }

  if (window->glfw_win != NULL) {
    glfwDestroyWindow(window->glfw_win);
  }

  glfwTerminate();
  free(window);
}

bool add_glyph_to_buffer(window_t* window, glyph_t op) {
  if (window->glyph_count >= GLYPH_BUFFER_SIZE) {
    // no more room for draw ops
    return false;
  }

  window->glyph_buffer[window->glyph_count++] = op;

  set_state_dirty(window);

  return true;
}

void begin_loop(window_t* window) {
  shader_program_t glyph_shader = create_glyph_shader();

  GLFWwindow* w = (GLFWwindow*)window->glfw_win;
  while (!glfwWindowShouldClose(w) && !window->quitting) {
    glfwWaitEventsTimeout(0.5);

    if (window->state == WINDOW_STATE_DIRTY) {
      glClearColor(0.0, 0.0, 0.0, 1.0);
      glClear(GL_COLOR_BUFFER_BIT);

      if (window->glyph_count > 0 && glyph_shader.valid) {
        draw_glyphs(&glyph_shader, window, window->glyph_buffer,
                    window->glyph_count);
        clear_glyph_buffer(window);
      }

      glfwSwapBuffers(w);
      set_state_wait(window);
    }
  }

  destroy_shader_program(&glyph_shader);
}

