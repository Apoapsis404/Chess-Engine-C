#ifndef LUTIL_H
#define LUTIL_H

#include <stdbool.h>

// Functions
void enable_module(const char* module);
void disable_module(const char* module);

typedef enum { DEBUG, INFO, WARNING, ERROR } log_level_t;
void set_log_level(log_level_t level);

void init_logging(const char* filename, bool clear_file);
void close_logging();

void load_config(const char* config_file);

void log_empty_line();
void log_message(log_level_t level, const char* module, const char* text);
void logf_message(log_level_t level, const char* module, const char* message, void *extra);

#endif //LUTIL_H;