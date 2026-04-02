#include "cfgutil.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static void trim_whitespace(char *str) {
    if (!str) return;
    // trim left
    char *start = str;
    while (*start && isspace((unsigned char)*start)) start++;
    if (start != str) memmove(str, start, strlen(start) + 1);

    // trim right
    char *end = str + strlen(str);
    while (end > str && isspace((unsigned char)*(end - 1))) end--;
    *end = '\0';
}

static void apply_config(const char *key, const char *value,
                         const cfg_entry_t *entries, size_t entry_count) {
    for (size_t i = 0; i < entry_count; ++i) {
        if (strcmp(key, entries[i].key) == 0) {
            entries[i].handler(value);
            return;
        }
    }
    // Unknown key; ignore to allow extensibility
}

void load_config_section(
    const char *config_file,
    const char *section_start,
    const char *section_end,
    const cfg_entry_t *entries,
    size_t entry_count) {

    if (!config_file || !section_start || !section_end) {
        fprintf(stderr, "load_config_section: invalid argument\n");
        return;
    }

    FILE *file = fopen(config_file, "r");
    if (!file) {
        fprintf(stderr, "Failed to open config file: %s\n", config_file);
        return;
    }

    bool in_config = false;
    char line[256];
    
    while (fgets(line, sizeof(line), file)) {
        char buffer[256];
        strncpy(buffer, line, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';

        trim_whitespace(buffer);
        if (buffer[0] == '\0') continue;  // empty line

        if (strncmp(buffer, section_start, strlen(section_start)) == 0) {
            in_config = true;
            continue;
        }

        if (strncmp(buffer, section_end, strlen(section_end)) == 0) {
            break;
        }

        if (!in_config) continue;

        if (buffer[0] == '#') continue;

        char *equals = strchr(buffer, '=');
        if (!equals) continue;

        *equals = '\0';
        char *key = buffer;
        char *value = equals + 1;

        trim_whitespace(key);
        trim_whitespace(value);

        if (key[0] == '\0' || value[0] == '\0') continue;

        apply_config(key, value, entries, entry_count);
    }

    fclose(file);
}
