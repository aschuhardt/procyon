#include <log.h>

#include "config.h"
#include "script.h"
#include "window.h"

int main(int argc, const char** argv) {
  config_t config;
  if (!parse_args(argc, argv, &config)) {
    return -1;
  }

  // create window object
  window_t* window = create_window(&config);
  if (window == NULL) {
    log_error("Failed to create window");
  } else {
    // create script environment object
    script_env_t* script_env = create_script_env(window);
    if (script_env == NULL) {
      log_error("Failed to create script environment object");
    }

    // load scripts from disk according to configuration settings
    if (load_scripts(script_env, config.script_mode, config.script_entry) !=
        LOAD_SUCCESS) {
      log_error("Failed to load script from %s", config.script_entry);
    }

    // start display loop
    begin_loop(window);

    destroy_script_env(script_env);
  }

  destroy_window(window);
  return 0;
}
