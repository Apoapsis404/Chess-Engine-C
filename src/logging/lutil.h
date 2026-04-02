#ifndef LUTIL_H
#define LUTIL_H

#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <stdarg.h>

// Functions

typedef enum { DEBUG, INFO, WARNING, ERROR, FATAL } log_level_t;
void set_log_level(log_level_t level);
void set_log_level_from_string(const char* level);

void init_logging(const char* filename, bool clear_file);
void close_logging();

void load_config(const char* config_file);

void log_empty_line();
void log_message(log_level_t level, const char* module, const char* text);
void logf_message(log_level_t level, const char* module, const char* message, ...);

//TODO: Make possible to pass string
void log_time_start(log_level_t level, const char* module, clock_t* time_it);
void log_time_stop(log_level_t level, const char* module, clock_t* time_it);

#endif //LUTIL_H;