#include <log.h>
#include <procyon.h>

#define FPS_AVG_BUFFER_SIZE 32
#define TEST_COUNT 19

typedef enum {
  TEST_MODE_STRING,
  TEST_MODE_RECT,
  TEST_MODE_LINE,
  TEST_COMPLETE
} test_mode_t;

typedef struct bench_state_t {
  double avg_buffer[FPS_AVG_BUFFER_SIZE];
  size_t avg_buffer_index, test_index, test_draw_counts[TEST_COUNT];
  test_mode_t test_mode;
  procy_window_t* window;
  double** test_results_by_mode;
} bench_state_t;

void on_load(procy_state_t* state) {
  bench_state_t* data = (bench_state_t*)state->data;
  data->avg_buffer_index = 0;
  data->test_index = 0;
  data->window->high_fps = true;
  data->test_mode = TEST_MODE_STRING;
  data->test_results_by_mode = malloc(sizeof(double*) * TEST_COMPLETE);

  for (int i = 0; i < TEST_COMPLETE; ++i) {
    data->test_results_by_mode[i] = malloc(sizeof(double) * TEST_COUNT);
  }

  // zero-out fps avg buffer
  for (int i = 0; i < FPS_AVG_BUFFER_SIZE; ++i) {
    data->avg_buffer[i] = 0.0;
  }

  // compute draw counts for each test
  for (int i = 0; i < TEST_COUNT; ++i) {
    data->test_draw_counts[i] = (size_t)1 << i;
  }
}

void on_unload(procy_state_t* state) {
  bench_state_t* data = (bench_state_t*)state->data;
  for (int i = 0; i < TEST_COMPLETE; ++i) {
    for (int j = 0; j < TEST_COUNT; ++j) {
      log_info("(Test %d) %zu draws => avg. %f FPS", i,
               data->test_draw_counts[j],
               1.0 / data->test_results_by_mode[i][j]);
    }

    free(data->test_results_by_mode[i]);
  }

  free(data->test_results_by_mode);
}

void on_draw(procy_state_t* state, double time) {
  const procy_color_t yellow = procy_create_color(1.0F, 1.0F, 0.0F);
  const procy_color_t black = procy_create_color(0.0F, 0.0F, 0.0F);

  bench_state_t* data = (bench_state_t*)state->data;
  data->avg_buffer[data->avg_buffer_index++] = time;

  if (data->avg_buffer_index > FPS_AVG_BUFFER_SIZE) {
    double avg = 0.0;
    for (int i = 0; i < FPS_AVG_BUFFER_SIZE; ++i) {
      avg += data->avg_buffer[i];
    }
    data->test_results_by_mode[data->test_mode][data->test_index++] =
        avg / (double)FPS_AVG_BUFFER_SIZE;
    data->avg_buffer_index = 0;
  }

  if (data->test_index >= TEST_COUNT) {
    if (++data->test_mode == TEST_COMPLETE) {
      procy_close_window(data->window);
      return;
    }

    data->test_index = 0;
  }

  for (size_t i = 0; i < data->test_draw_counts[data->test_index]; ++i) {
    switch (data->test_mode) {
      case TEST_MODE_STRING:
        procy_draw_string(data->window, 200 + (i * 2 % 200),
                          200 + (i * 3 % 200), yellow, black, "butts");
        break;
      case TEST_MODE_RECT:
        procy_draw_rect(data->window, 200 + (i * 2 % 200), 200 + (i * 3 % 200),
                        30, 30, yellow);
        break;
      case TEST_MODE_LINE:
        procy_draw_line(data->window, 200 + (i * 2 % 200), 200 + (i * 3 % 200),
                        0, 0, yellow);
        break;
      case TEST_COMPLETE:
        break;
    }
  }

  char fps_text[32];
  sprintf(fps_text, "FPS: %.2f", 1.0 / time);
  procy_draw_string(data->window, 0, 0, yellow, black, fps_text);

  char draw_count_text[32];
  sprintf(draw_count_text, "Count: %zu",
          data->test_draw_counts[data->test_index]);
  procy_draw_string(data->window, 0, 10, yellow, black, draw_count_text);
}

int main(int argc, const char** argv) {
  bench_state_t data;
  procy_state_t* state = procy_create_callback_state(
      on_load, on_unload, on_draw, NULL, NULL, NULL, NULL);
  state->data = &data;
  procy_window_t* window =
      procy_create_window(800, 600, "Framerate Benchmark", 1.0f, state);
  data.window = window;
  procy_begin_loop(window);
  procy_destroy_window(window);
  procy_destroy_state(state);
  return 0;
}
