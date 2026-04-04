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
void set_log_args(int argc, const char **argv);
void set_log_file(const char *filename);
void set_log_entity_limit(int max_entities);

void init_logging();
void close_logging();

void load_config(const char* config_file);

void log_empty_line();
void log_message(log_level_t level, const char* module, const char* text);
void logf_message(log_level_t level, const char* module, const char* message, ...);

#define TODO log_todo(__func__, __FILE__, __LINE__)
void log_todo(const char *func_name, const char *file_name, int line_number);

void log_header(int argc, const char **argv);
void log_footer(void);

//TODO: Make possible to pass string
void log_time_start(log_level_t level, const char* module, clock_t* time_it);
void log_time_stop(log_level_t level, const char* module, clock_t* time_it);

#endif //LUTIL_H;