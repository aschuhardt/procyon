#include "window.h"

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
// clang-format on

#include <limits.h>
#include <log.h>
#include <string.h>

#include "color.h"
#include "drawing.h"
#include "keys.h"
#include "mouse.h"
#include "shader.h"
#include "shader/error.h"
#include "shader/frame.h"
#include "shader/glyph.h"
#include "shader/line.h"
#include "shader/rect.h"
#include "shader/sprite.h"
#include "state.h"

#define DEFAULT_SCALE 1.0f

typedef procy_window_t window_t;
typedef procy_glyph_shader_program_t glyph_shader_program_t;
typedef procy_rect_shader_program_t rect_shader_program_t;
typedef procy_line_shader_program_t line_shader_program_t;
typedef procy_sprite_shader_program_t sprite_shader_program_t;
typedef procy_draw_op_text_t draw_op_text_t;
typedef procy_draw_op_rect_t draw_op_rect_t;
typedef procy_draw_op_sprite_t draw_op_sprite_t;
typedef procy_draw_op_line_t draw_op_line_t;
typedef procy_key_info_t key_info_t;
typedef procy_color_t color_t;
typedef procy_state_t state_t;

// associates a sprite shader with all of the pending draw-ops that correspond
// to it
typedef struct procy_draw_op_sprite_bucket_t {
  sprite_shader_program_t *shader;
  draw_op_sprite_t *sprite_draw_ops;
} procy_draw_op_sprite_bucket_t;

typedef procy_draw_op_sprite_bucket_t draw_op_sprite_bucket_t;

static void glfw_error_callback(int code, const char *msg) {
  log_error("GLFW error %d: %s", code, msg);
}

static void set_gl_hints(void) {
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

  procy_frame_shader_resized(window->shaders.frame, width, height);

  set_ortho_projection(window, width, height);
  state_t *state = window->state;
  if (state->on_resize != NULL) {
    state->on_resize(state, width, height);
  }
}

static void mouse_moved(GLFWwindow *w, double x, double y) {
  window_t *window = (window_t *)glfwGetWindowUserPointer(w);
  state_t *state = window->state;
  if (state->on_mouse_moved != NULL) {
    state->on_mouse_moved(state, x, y);
  }
}

static void mouse_action(GLFWwindow *w, int button, int action, int mods) {
  window_t *window = (window_t *)glfwGetWindowUserPointer(w);
  state_t *state = window->state;
  procy_mouse_button_t mapped_button = procy_map_glfw_mouse_button(button);
  bool shift = (mods & GLFW_MOD_SHIFT) == GLFW_MOD_SHIFT;
  bool ctrl = (mods & GLFW_MOD_CONTROL) == GLFW_MOD_CONTROL;
  bool alt = (mods & GLFW_MOD_ALT) == GLFW_MOD_ALT;
  if (action == GLFW_PRESS && state->on_mouse_pressed != NULL) {
    state->on_mouse_pressed(state, mapped_button, shift, ctrl, alt);
  } else if (action == GLFW_RELEASE && state->on_mouse_released != NULL) {
    state->on_mouse_released(state, mapped_button, shift, ctrl, alt);
  }
}

static void destroy_sprite_shaders(window_t *window) {
  // first clean-up sprite draw-op buckets that refer to the shaders
  // now that no buckets are referring to the shaders, we can clean them up as
  // well
  sprite_shader_program_t **sprite_shaders = window->shaders.sprite;
  for (int i = 0; i < arrlen(sprite_shaders); ++i) {
    procy_destroy_sprite_shader(sprite_shaders[i]);
  }

  arrfree(sprite_shaders);
}

static void destroy_shaders(window_t *window) {
  procy_destroy_frame_shader(window->shaders.frame);
  procy_destroy_glyph_shader(window->shaders.glyph);
  procy_destroy_rect_shader(window->shaders.rect);
  procy_destroy_line_shader(window->shaders.line);
  destroy_sprite_shaders(window);
}

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

  glViewport(0, 0, width, height);

  return true;
}

static void key_entered(GLFWwindow *w, int key, int scancode, int action,
                        int mods) {
  window_t *window = glfwGetWindowUserPointer(w);
  bool shift = (mods & GLFW_MOD_SHIFT) == GLFW_MOD_SHIFT;
  bool ctrl = (mods & GLFW_MOD_CONTROL) == GLFW_MOD_CONTROL;
  bool alt = (mods & GLFW_MOD_ALT) == GLFW_MOD_ALT;
  state_t *state = window->state;
  if ((action == GLFW_PRESS || action == GLFW_REPEAT) &&
      state->on_key_pressed != NULL) {
    if (key == GLFW_KEY_F11) {
      procy_set_fullscreen(window);
    }
    state->on_key_pressed(state, window->key_table[key], shift, ctrl, alt);
  } else if (action == GLFW_RELEASE && state->on_key_released != NULL) {
    state->on_key_released(state, window->key_table[key], shift, ctrl, alt);
  }
}

static void char_entered(GLFWwindow *w, unsigned int codepoint) {
  window_t *window = glfwGetWindowUserPointer(w);
  state_t *state = window->state;
  if (state->on_char_entered != NULL) {
    state->on_char_entered(state, codepoint);
  }
}

static void set_event_callbacks(window_t *w) {
  glfwSetKeyCallback(w->glfw_win, key_entered);
  glfwSetCharCallback(w->glfw_win, char_entered);
  glfwSetFramebufferSizeCallback(w->glfw_win, window_resized);
  glfwSetCursorPosCallback(w->glfw_win, mouse_moved);
  glfwSetMouseButtonCallback(w->glfw_win, mouse_action);
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

static void init_shaders(window_t *window) {
  window->shaders.frame = procy_create_frame_shader(window);
  window->shaders.glyph = procy_create_glyph_shader();
  window->shaders.rect = procy_create_rect_shader();
  window->shaders.line = procy_create_line_shader();
}

static void log_opengl_info(void) {
  log_debug("Vendor: %s", glGetString(GL_VENDOR));
  log_debug("Renderer: %s", glGetString(GL_RENDERER));
  log_debug("OpenGL Version: %s", glGetString(GL_VERSION));
}

static void set_dpi_scale(window_t *window) {
  GLFWmonitor *monitor = glfwGetWindowMonitor(window->glfw_win);
  if (monitor != NULL) {
    // full-screen mode; use the monitor's DPI
    glfwGetMonitorContentScale(monitor, &window->dpi_scale.x,
                               &window->dpi_scale.y);
  } else {
    // windowed; use the window DPI
    glfwGetWindowContentScale(window->glfw_win, &window->dpi_scale.x,
                              &window->dpi_scale.y);
  }

  log_debug("DPI scale: %0.2fx%0.2f", window->dpi_scale.x, window->dpi_scale.y);
}

window_t *procy_create_window(int width, int height, const char *title,
                              state_t *state) {
  glfwSetErrorCallback(glfw_error_callback);
  window_t *window = calloc(1, sizeof(window_t));

  if (set_gl_window_pointer(window, width, height, title)) {
    log_opengl_info();

    window->initial_size.width = width;
    window->initial_size.height = height;

    window->state = state;
    window->scale = DEFAULT_SCALE;

    init_shaders(window);
    init_key_table(window);
    set_ortho_projection(window, width, height);
    set_event_callbacks(window);
    set_dpi_scale(window);
  } else {
    log_error("Failed to initialize window or OpenGL context!");

    free(window);
    return NULL;
  }

  return window;
}

static void draw_sprite_shaders(window_t *window) {
  for (int i = 0; i < arrlen(window->draw_ops_sprite); ++i) {
    draw_op_sprite_bucket_t *bucket = &window->draw_ops_sprite[i];
    if (arrlen(bucket->sprite_draw_ops) > 0) {
      procy_draw_sprite_shader(bucket->shader, window, bucket->sprite_draw_ops);
    }
  }
}

void procy_append_sprite_shader(procy_window_t *window,
                                struct procy_sprite_shader_program_t *shader) {
  arrput(window->shaders.sprite, shader);  // NOLINT
}

void procy_destroy_window(window_t *window) {
  if (window == NULL) {
    return;
  }

  for (int i = 0; i < arrlen(window->draw_ops_sprite); ++i) {
    arrfree(window->draw_ops_sprite[i].sprite_draw_ops);
  }

  arrfree(window->draw_ops_sprite);
  arrfree(window->draw_ops_rect);
  arrfree(window->draw_ops_text);
  arrfree(window->draw_ops_line);

  destroy_shaders(window);

  if (window->glfw_win != NULL) {
    glfwDestroyWindow(window->glfw_win);
  }

  if (window->key_table != NULL) {
    free(window->key_table);
  }

  glfwTerminate();
  free(window);
}

void procy_append_draw_op_text(procy_window_t *window, draw_op_text_t *op) {
  arrput(window->draw_ops_text, *op);
}

void procy_append_draw_op_rect(procy_window_t *window, draw_op_rect_t *op) {
  arrput(window->draw_ops_rect, *op);
}

void procy_append_draw_op_sprite(procy_window_t *window, draw_op_sprite_t *op) {
  for (int i = 0; i < arrlen(window->draw_ops_sprite); ++i) {
    draw_op_sprite_bucket_t *bucket = &window->draw_ops_sprite[i];
    if (bucket->shader == op->ptr->shader) {
      arrput(bucket->sprite_draw_ops, *op);
      return;
    }
  }

  // no draw ops for this shader have been created yet; create a new bucket to
  // hold this op and its sprite's shader
  draw_op_sprite_bucket_t bucket = {op->ptr->shader, NULL};
  arrput(bucket.sprite_draw_ops, *op);
  arrput(window->draw_ops_sprite, bucket);
}

void procy_append_draw_op_line(procy_window_t *window, draw_op_line_t *op) {
  arrput(window->draw_ops_line, *op);
}

void procy_get_window_size(window_t *window, int *width, int *height) {
  glfwGetWindowSize(window->glfw_win, width, height);
}

void procy_get_glyph_size(procy_window_t *window, int *width, int *height) {
  // assume that we're always going to have our default glyph shader in the
  // first index
  procy_get_glyph_bounds(window->shaders.glyph, width, height);

  if (width != NULL) {
    *width = (int)((float)*width * window->scale);
  }

  if (height != NULL) {
    *height = (int)((float)*height * window->scale);
  }
}

static void execute_draw_ops(window_t *window) {
  // bind the framebuffer so that all draw ops are drawn to its texture instead
  // of directly to the screen
  procy_frame_shader_begin(window->shaders.frame);

  procy_draw_rect_shader(window->shaders.rect, window, window->draw_ops_rect);
  procy_draw_line_shader(window->shaders.line, window, window->draw_ops_line);
  procy_draw_glyph_shader(window->shaders.glyph, window, window->draw_ops_text);
  draw_sprite_shaders(window);

  // un-bind the framebuffer
  procy_frame_shader_end(window->shaders.frame);
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
  while (!glfwWindowShouldClose(w) && !window->quitting) {
    double current_time = glfwGetTime();
    double frame_duration = current_time - last_frame_time;
    last_frame_time = current_time;

    if (window->high_fps) {
      glfwPollEvents();
    } else {
      glfwWaitEventsTimeout(1.0);
    }

    if (state->on_draw != NULL) {
      state->on_draw(state, frame_duration);
    }

    execute_draw_ops(window);

    procy_draw_frame_shader(window->shaders.frame);

    glfwSwapBuffers(w);
  }

  if (state->on_unload != NULL) {
    state->on_unload(state);
  }
}

void procy_set_clear_color(color_t c) {
  unsigned char r;
  unsigned char g;
  unsigned char b;

  procy_get_color_rgb(&c, &r, &g, &b);

  glClearColor((float)r / 255.0F, (float)g / 255.0F, (float)b / 255.0F, 1.0F);
}

void procy_close_window(procy_window_t *window) { window->quitting = true; }

void procy_set_high_fps_mode(procy_window_t *window, bool high_fps) {
  window->high_fps = high_fps;
}

void procy_set_window_title(procy_window_t *window, const char *title) {
  glfwSetWindowTitle(window->glfw_win, title);
}

void procy_set_scale(window_t *window, float scale) {
  window->scale = scale;
  int width;
  int height;
  procy_get_window_size(window, &width, &height);
  set_ortho_projection(window, width, height);
}

void procy_reset_scale(procy_window_t *window) {
  window->scale = DEFAULT_SCALE;
  int width;
  int height;
  procy_get_window_size(window, &width, &height);
  set_ortho_projection(window, width, height);
}

void procy_set_mouse_captured(window_t *window) {
  glfwSetInputMode(window->glfw_win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void procy_set_mouse_normal(window_t *window) {
  glfwSetInputMode(window->glfw_win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void procy_set_mouse_hidden(window_t *window) {
  glfwSetInputMode(window->glfw_win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

void procy_get_mouse_position(window_t *window, double *x, double *y) {
  glfwGetCursorPos(window->glfw_win, x, y);
}

void procy_set_fullscreen(procy_window_t *window) {
  GLFWmonitor *monitor = glfwGetPrimaryMonitor();
  if (monitor != NULL) {
    log_debug("Switching to fullscreen mode");
    const GLFWvidmode *video_mode = glfwGetVideoMode(monitor);
    glfwSetWindowMonitor(window->glfw_win, monitor, 0, 0, video_mode->width,
                         video_mode->height, GLFW_DONT_CARE);
  }

  set_dpi_scale(window);
}

void procy_set_windowed(procy_window_t *window) {
  GLFWmonitor *monitor = glfwGetWindowMonitor(window->glfw_win);
  if (monitor == NULL) {
    // already in windowed mode
    return;
  }

  log_debug("Switching to windowed mode");
  const GLFWvidmode *video_mode = glfwGetVideoMode(monitor);
  glfwSetWindowMonitor(window->glfw_win, NULL,
                       video_mode->width / 2 - window->initial_size.width / 2,
                       video_mode->height / 2 - window->initial_size.height / 2,
                       window->initial_size.width, window->initial_size.height,
                       GLFW_DONT_CARE);

  set_dpi_scale(window);
}
