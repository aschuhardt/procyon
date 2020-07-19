#include "config.h"

#include <log.h>
#include <string.h>

#include "argparse.h"

static const char* usage[] = {
    "procyon [--(log level)] [-e|--entry (file path)]\n"
    "Log levels: error (default), warn, info, debug, trace",
    NULL};

static const char DEFAULT_SCRIPT_PATH[] = "script/main.lua";

static const int DEFAULT_WINDOW_W = 800;
static const int DEFAULT_WINDOW_H = 600;

bool parse_config_args(int argc, const char** argv, config_t* cfg) {
  // setup available command-line arguments and their descriptions
  int log_level = 0;
  const char* entry_path = NULL;

  cfg->window_w = DEFAULT_WINDOW_W;
  cfg->window_h = DEFAULT_WINDOW_H;
  cfg->glyph_scale = 1.0F;

  memset(cfg->script_entry, '\0', sizeof(cfg->script_entry) / sizeof(char));

  struct argparse_option options[] = {
      OPT_GROUP("General"),
      OPT_HELP(),
      OPT_GROUP("Logging"),
      OPT_BIT(0, "error", &log_level, "enable ERROR log level", NULL, LOG_ERROR,
              OPT_NONEG),
      OPT_BIT(0, "warn", &log_level, "enable WARN log level", NULL, LOG_WARN,
              OPT_NONEG),
      OPT_BIT(0, "info", &log_level, "enable INFO log level", NULL, LOG_INFO,
              OPT_NONEG),
      OPT_BIT(0, "debug", &log_level, "enable DEBUG log level", NULL, LOG_DEBUG,
              OPT_NONEG),
      OPT_BIT(0, "trace", &log_level, "enable TRACE log level", NULL, LOG_TRACE,
              OPT_NONEG),
      OPT_GROUP("Script loading"),
      OPT_STRING('e', "entry", &entry_path,
                 "script entry point (default = 'script/main.lua')"),
      OPT_GROUP("Visuals"),
      OPT_INTEGER('w', "width", &cfg->window_w, "window width"),
      OPT_INTEGER('h', "height", &cfg->window_h, "window height"),
      OPT_FLOAT('s', "scale", &cfg->glyph_scale, "text scale (default = 1.0)"),
      OPT_END(),
  };

  // setup parser and feed it the argument info and incoming values
  struct argparse argparse;
  argparse_init(&argparse, options, usage, 0);
  argparse_describe(
      &argparse,
      "A GPU-accelerated tile-based game engine with scripting via Lua",
      "Created by Addison Schuhardt");

  // parse arguments
  argparse_parse(&argparse, argc, argv);

  // set log level
  log_set_level(log_level ? log_level : LOG_ERROR);

  // ensure that an entry point was provided and that it's of a valid length
  size_t max_path_length = sizeof(cfg->script_entry) / sizeof(char);
  if (entry_path == NULL) {
    memcpy(cfg->script_entry, DEFAULT_SCRIPT_PATH, sizeof(DEFAULT_SCRIPT_PATH));
  } else if (strnlen(entry_path, max_path_length) < max_path_length) {
    memcpy(cfg->script_entry, entry_path, strnlen(entry_path, max_path_length));
  } else {
    log_error(
        "The script entry path must be fewer than %zu characters in length",
        max_path_length);
    return false;
  }

  log_debug("Script entry path: %s", cfg->script_entry);

  return true;
}
