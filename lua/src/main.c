#include <log.h>

#include "config.h"
#include "procyon.h"
#include "script.h"

int main(int argc, const char** argv) {
  config_t config;
  if (!parse_config_args(argc, argv, &config)) {
    return -1;
  }

  procy_state_t state;
  state.children = NULL;
  state.child_count = 0;
  state.data = NULL;

  bool reload = false;
  do {
    // create window object
    procy_window_t* window =
        procy_create_window(config.window_w, config.window_h, "Procyon Lua",
                            config.glyph_scale, &state);
    if (window == NULL) {
      log_error("Failed to create window");
    } else {
      // create script environment object
      script_env_t* script_env = create_script_env(window, &state);
      state.data = script_env;
      if (script_env == NULL) {
        log_error("Failed to create script environment object");
      } else {
        // load scripts from disk according to configuration settings
        if (!load_scripts(script_env, config.script_entry)) {
          log_error("Failed to load script from %s", config.script_entry);
        }

        // start display loop
        procy_begin_loop(window);

        // should we restart?
        reload = script_env->reload;

        destroy_script_env(script_env);
      }
    }

    procy_destroy_window(window);
  } while (reload);

  return 0;
}
