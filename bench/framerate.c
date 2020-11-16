#include <log.h>
#include <procyon.h>

#define FPS_AVG_BUFFER_SIZE 32
#define TEST_COUNT 20

typedef struct bench_state_t {
  double avg_buffer[FPS_AVG_BUFFER_SIZE], test_results[TEST_COUNT];
  size_t avg_buffer_index, test_index, test_draw_counts[TEST_COUNT];
  procy_window_t* window;
} bench_state_t;

void on_load(procy_state_t* state) {
  bench_state_t* data = (bench_state_t*)state->data;
  data->avg_buffer_index = 0;
  data->test_index = 0;
  data->window->high_fps = true;

  // zero-out fps avg buffer
  for (int i = 0; i < FPS_AVG_BUFFER_SIZE; ++i) {
    data->avg_buffer[i] = 0.0;
  }

  // compute draw counts for each test
  for (int i = 0; i < TEST_COUNT; ++i) {
    data->test_results[i] = 0.0;
    data->test_draw_counts[i] = 1 << i;
  }
}

void on_unload(procy_state_t* state) {
  bench_state_t* data = (bench_state_t*)state->data;
  for (int i = 0; i < TEST_COUNT; ++i) {
    log_info("%zu draws => avg. %f FPS", data->test_draw_counts[i],
             1.0 / data->test_results[i]);
  }
}

void on_draw(procy_state_t* state, double time) {
  bench_state_t* data = (bench_state_t*)state->data;
  data->avg_buffer[data->avg_buffer_index++] = time;

  if (data->avg_buffer_index > FPS_AVG_BUFFER_SIZE) {
    double avg = 0.0;
    for (int i = 0; i < FPS_AVG_BUFFER_SIZE; ++i) {
      avg += data->avg_buffer[i];
    }
    data->test_results[data->test_index++] = avg / (double)FPS_AVG_BUFFER_SIZE;
    data->avg_buffer_index = 0;
  }

  if (data->test_index >= TEST_COUNT) {
    procy_close_window(data->window);
    return;
  }

  for (int i = 0; i < data->test_draw_counts[data->test_index]; ++i) {
    procy_draw_string(data->window, 200 + (i * 2 % 200), 200 + (i * 3 % 200),
                      procy_create_color(1.0f, 1.0f, 0.0f),
                      procy_create_color(0.0f, 0.0f, 0.2f), "butts");
  }

  char fps_text[32];
  sprintf(fps_text, "%.2f", 1.0 / time);
  procy_draw_string(data->window, 0, 0, procy_create_color(0.0f, 1.0f, 0.0f),
                    procy_create_color(0.0f, 0.0f, 0.0f), fps_text);
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
  procy_destroy_callback_state(state);
  return 0;
}
