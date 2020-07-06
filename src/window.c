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

static void set_ortho_projection(window_t* window) {
  int width = window->bounds.width;
  int height = window->bounds.height;

  memset(&window->ortho[0][0], 0, 16);

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

  set_state_dirty(window);
}

static void set_window_callbacks(GLFWwindow* w) {
  glfwSetFramebufferSizeCallback(w, window_resized);
}

static void clear_glyph_buffer(window_t* window) { window->glyphs.count = 0; }

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

  set_ortho_projection(window);
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
  if (window->glyphs.count >= GLYPH_BUFFER_SIZE) {
    // no more room for draw ops
    return false;
  }

  window->glyphs.buffer[window->glyphs.count++] = op;
  set_state_dirty(window);
  return true;
}

void begin_loop(window_t* window) {
  glyph_shader_program_t glyph_shader = create_glyph_shader();

  GLFWwindow* w = (GLFWwindow*)window->glfw_win;
  while (!glfwWindowShouldClose(w) && !window->quitting) {
    glfwWaitEventsTimeout(0.5F);

    if (window->state == WINDOW_STATE_DIRTY) {
      glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
      glClear(GL_COLOR_BUFFER_BIT);

      if (window->glyphs.count > 0 && glyph_shader.program.valid) {
        draw_glyphs(&glyph_shader, window, window->glyphs.buffer,
                    window->glyphs.count);
        clear_glyph_buffer(window);
      }

      glfwSwapBuffers(w);
      set_state_wait(window);
    }
  }

  destroy_glyph_shader_program(&glyph_shader);
}

