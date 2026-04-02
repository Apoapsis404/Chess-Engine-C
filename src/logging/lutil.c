#include "lutil.h"
#include "../util/cfgutil.h"

#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#define MAX_MODULES 10
#define MODULE_NAME_LENGTH 20

typedef struct log_t {
    FILE *log_file;
    char *log_filename;

    log_level_t current_log_level;
    bool clear_file;

    int max_entities;

    const char *config_file;

    // Program args
    int argc;
    const char **argv;
} log_t;

static log_t log = {
    .log_file = NULL,
    .log_filename = "",

    .current_log_level = DEBUG,
    .clear_file = true,
    .max_entities = 0,

    .config_file = "",

    .argc = 0,
    .argv = NULL,
};

void set_log_file(const char *filename) {
    if (!filename) return;
    // Make a copy to avoid dangling pointer issues
    log.log_filename = strdup(filename);
}

// Config handlers
static void handle_log_level(const char *value) {
    if (strcmp(value, "DEBUG") == 0) set_log_level(DEBUG);
    else if (strcmp(value, "INFO") == 0) set_log_level(INFO);
    else if (strcmp(value, "WARNING") == 0) set_log_level(WARNING);
    else if (strcmp(value, "ERROR") == 0) set_log_level(ERROR);
    else if (strcmp(value, "FATAL") == 0) set_log_level(FATAL);
}

static void handle_clear_file(const char *value) {
    if (strcmp(value, "false") == 0) log.clear_file = false;
    else if (strcmp(value, "true") == 0) log.clear_file = true;
}

static void handle_max_log_entities(const char *value) {
    int max_ent = atoi(value);
    set_log_entity_limit(max_ent);
}

static void handle_log_file(const char *value) {
    set_log_file(value);
}

// Config
void load_config(const char* config_file) {

    if (config_file == NULL) {
        init_logging();
        return;
    }

    static const cfg_entry_t log_config_entries[] = {
        {"log_level", handle_log_level},
        {"log_file", handle_log_file},
        {"clear_file", handle_clear_file},
        {"max_log_entities", handle_max_log_entities},
    };

    load_config_section(
        config_file,
        "# LOG CONFIG START",
        "# LOG CONFIG END",
        log_config_entries,
        sizeof(log_config_entries) / sizeof(log_config_entries[0])
    );

    init_logging();
}

//Log Level
void set_log_level(log_level_t level){
    log.current_log_level = level;
}

void set_log_args(int argc, const char **argv) {
    log.argc = argc;
    log.argv = argv;
}

void set_log_level_from_string(const char* level){
    if (strcmp(level , "DEBUG") == 0) set_log_level(DEBUG); 
    else if (strcmp(level , "INFO") == 0) set_log_level(INFO); 
    else if (strcmp(level , "WARNING") == 0) set_log_level(WARNING); 
    else if (strcmp(level , "ERROR") == 0) set_log_level(ERROR); 
    else if (strcmp(level , "FATAL") == 0) set_log_level(FATAL); 
}

void set_log_entity_limit(int max_entities){
    if(max_entities < 0) {
        max_entities = 0;
    }
    log.max_entities = max_entities;
}

static bool is_log_start_line(const char *line) {
    return line && strstr(line, "LOG START") != NULL;
}

static void trim_log_file_to_entity_limit(void) {
    if (!log.log_filename || log.log_filename[0] == '\0') return;
    if (log.clear_file) return;
    if (log.max_entities <= 0) return;

    FILE *input = fopen(log.log_filename, "r");
    if (!input) return;

    char **lines = NULL;
    size_t lines_cap = 0;
    size_t lines_count = 0;
    size_t *entity_starts = NULL;
    size_t entity_cap = 0;
    size_t entity_count = 0;

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), input)) {
        char *line = strdup(buffer);
        if (!line) continue;

        if (lines_count >= lines_cap) {
            size_t new_cap = lines_cap ? lines_cap * 2 : 256;
            char **new_lines = realloc(lines, new_cap * sizeof(char*));
            if (!new_lines) {
                free(line);
                break;
            }
            lines = new_lines;
            lines_cap = new_cap;
        }
        lines[lines_count++] = line;

        if (is_log_start_line(buffer)) {
            if (entity_count >= entity_cap) {
                size_t new_cap = entity_cap ? entity_cap * 2 : 16;
                size_t *new_entity = realloc(entity_starts, new_cap * sizeof(size_t));
                if (!new_entity) break;
                entity_starts = new_entity;
                entity_cap = new_cap;
            }
            entity_starts[entity_count++] = lines_count - 1;
        }
    }
    fclose(input);

    if (entity_count <= (size_t)log.max_entities) {
        for (size_t i = 0; i < lines_count; ++i) free(lines[i]);
        free(lines);
        free(entity_starts);
        return;
    }

    size_t remove = entity_count - log.max_entities;
    size_t keep_start_line = entity_starts[remove];

    FILE *output = fopen(log.log_filename, "w");
    if (output) {
        for (size_t i = keep_start_line; i < lines_count; ++i) {
            fputs(lines[i], output);
        }
        fclose(output);
    }

    for (size_t i = 0; i < lines_count; ++i) free(lines[i]);
    free(lines);
    free(entity_starts);
}

//FILE

void log_header(int argc, const char **argv) {
    if (!log.log_file) {
        fprintf(stderr, "Logging not initialized.\n");
        return;
    }

    time_t now = time(NULL);
    struct tm* local_time = localtime(&now);

    fprintf(log.log_file, "==================== LOG START ====================\n");
    fprintf(log.log_file, "Timestamp: %04d-%02d-%02d %02d:%02d:%02d\n",
            local_time->tm_year + 1900, local_time->tm_mon + 1,
            local_time->tm_mday, local_time->tm_hour,
            local_time->tm_min, local_time->tm_sec);

    if (argc > 0 && argv) {
        fprintf(log.log_file, "Program arguments (%d):", argc);
        for (int i = 0; i < argc; ++i) {
            fprintf(log.log_file, " %s", argv[i] ? argv[i] : "(null)");
        }
        fprintf(log.log_file, "\n");
    } else {
        fprintf(log.log_file, "Program arguments: [none]\n");
    }

    fprintf(log.log_file, "===================================================\n");
    fflush(log.log_file);
}

void log_footer(void) {
    if (!log.log_file) {
        fprintf(stderr, "Logging not initialized.\n");
        return;
    }

    time_t now = time(NULL);
    struct tm* local_time = localtime(&now);

    fprintf(log.log_file, "===================== LOG END =====================\n");
    fprintf(log.log_file, "Timestamp: %04d-%02d-%02d %02d:%02d:%02d\n",
            local_time->tm_year + 1900, local_time->tm_mon + 1,
            local_time->tm_mday, local_time->tm_hour,
            local_time->tm_min, local_time->tm_sec);
    fprintf(log.log_file, "===================================================\n");
    fflush(log.log_file);
}

void init_logging(){
    if(!log.log_filename || log.log_filename[0] == '\0'){
        printf("[INFO] Logging on stdout!\n");
        log.log_file = stdout;
        log_header(log.argc, log.argv);
        return;
    }

    if (!log.clear_file && log.max_entities > 0) {
        trim_log_file_to_entity_limit();
    }

    if (log.clear_file) log.log_file = fopen(log.log_filename, "w");
    else log.log_file = fopen(log.log_filename, "a");
    if(!log.log_file){
        fprintf(stderr, "Failed to open log file: %s\n", log.log_filename);
        exit(EXIT_FAILURE);
    }
    log_empty_line();
    log_header(log.argc, log.argv);
}

void close_logging(){
    if (log.log_file) {
        log_footer();
    }
    if(log.log_file && log.log_file != stdout){
        fclose(log.log_file);
        log.log_file = NULL;
        free(log.log_filename);
        log.log_filename = NULL;
    }
}

//TIMING
void log_time_start(log_level_t level, const char* module, clock_t* time_it){
    *time_it = clock();
    log_message(level, module, "Starting timer");
}

void log_time_stop(log_level_t level, const char* module, clock_t* time_it){
    clock_t end = clock();

    double cpu_time_used = ((double) (end - *time_it)) / CLOCKS_PER_SEC;

    char buf[350];
    snprintf(buf, sizeof(buf), "Stopping timer. Time elapsed: %lf seconds", cpu_time_used);

    log_message(level, module, buf);
}

void log_empty_line(){
    if (!log.log_file) {
        //fprintf(stderr, "Logging not initialized.\n");
        printf("\n");
        return;
    }
    fprintf(log.log_file, "\n");
    fflush(log.log_file);
}

void logf_message(log_level_t level, const char* module, const char* message, ...){
    va_list args;
    va_start(args, message);
    
    const char* level_strings[] = { "DEBUG", "INFO", "WARNING", "ERROR", "FATAL" };
    time_t now = time(NULL);
    struct tm* local_time = localtime(&now);

    fprintf(log.log_file, "%04d-%02d-%02d %02d:%02d:%02d [%s][%s] ",
            local_time->tm_year + 1900, local_time->tm_mon+1,
            local_time->tm_mday, local_time->tm_hour, local_time->tm_min,
            local_time->tm_sec, level_strings[level], module);
    vfprintf(log.log_file, message, args);
    fprintf(log.log_file, "\n");
    fflush(log.log_file);
    va_end(args);
}

void log_message(log_level_t level, const char* module, const char* message){
    if(!module || !message){
        fprintf(stderr, "Error: Null parameter passed to log_message\n");
        return;
    }

    if(level < log.current_log_level){
        return;
    }

    if (!log.log_file) {
        fprintf(stderr, "Logging not initialized.\n");
        return;
    }

    const char* level_strings[] = { "DEBUG", "INFO", "WARNING", "ERROR", "FATAL" };
    time_t now = time(NULL);
    struct tm* local_time = localtime(&now);

    fprintf(log.log_file, "%04d-%02d-%02d %02d:%02d:%02d [%s][%s] %s\n",
            local_time->tm_year + 1900, local_time->tm_mon+1,
            local_time->tm_mday, local_time->tm_hour, local_time->tm_min,
            local_time->tm_sec, level_strings[level], module, message);
    
    fflush(log.log_file);
}

/*
    FOR JSON!
    fprintf(log.log_file, "{ \"timestamp\": %ld, \"level\": \"%s\", \"module\": \"%s\", \"message\": \"%s\" }\n",
            now, level_strings[level], module, text);
    */