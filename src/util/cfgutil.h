#ifndef CFGUTIL_H
#define CFGUTIL_H

#include <stddef.h>

typedef void (*cfg_handler_t)(const char *value);

typedef struct {
    const char *key;
    cfg_handler_t handler;
} cfg_entry_t;

void load_config_section(
    const char *config_file,
    const char *section_start,
    const char *section_end,
    const cfg_entry_t *entries,
    size_t entry_count
);

#endif // CFGUTIL_H
