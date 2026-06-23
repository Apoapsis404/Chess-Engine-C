#include "lutil.h"
#include "../util/cfgutil.h"

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

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

// Background logging queue
typedef struct log_message_node {
    char *text;
    struct log_message_node *next;
} log_message_node;

static log_message_node *queue_head = NULL;
static log_message_node *queue_tail = NULL;
static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;
static pthread_t log_thread;
static int stop_requested = 0;

static void enqueue_text(char *text) {
    if (!text) return;
    log_message_node *node = malloc(sizeof(*node));
    if (!node) { free(text); return; }
    node->text = text;
    node->next = NULL;

    pthread_mutex_lock(&queue_mutex);
    if (!queue_tail) {
        queue_head = queue_tail = node;
    } else {
        queue_tail->next = node;
        queue_tail = node;
    }
    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_mutex);
}

static char *format_full_message(log_level_t level, const char *module, const char *message_text) {
    const char* level_strings[] = { "DEBUG", "INFO", "WARNING", "ERROR", "FATAL" };
    time_t now = time(NULL);
    struct tm local_time;
    localtime_r(&now, &local_time);

    const char *mod = module ? module : "(null)";

    // Calculate required size
    int needed = snprintf(NULL, 0, "%04d-%02d-%02d %02d:%02d:%02d [%s][%s] %s\n",
                          local_time.tm_year + 1900, local_time.tm_mon+1,
                          local_time.tm_mday, local_time.tm_hour, local_time.tm_min,
                          local_time.tm_sec, level_strings[level], mod, message_text);
    if (needed < 0) return NULL;
    size_t size = (size_t)needed + 1;
    char *buf = malloc(size);
    if (!buf) return NULL;
    if (snprintf(buf, size, "%04d-%02d-%02d %02d:%02d:%02d [%s][%s] %s\n",
                 local_time.tm_year + 1900, local_time.tm_mon+1,
                 local_time.tm_mday, local_time.tm_hour, local_time.tm_min,
                 local_time.tm_sec, level_strings[level], mod, message_text) < 0) {
        free(buf);
        return NULL;
    }
    return buf;
}

static void *log_worker(void *arg) {
    (void)arg;
    for (;;) {
        pthread_mutex_lock(&queue_mutex);
        while (!queue_head && !stop_requested) {
            pthread_cond_wait(&queue_cond, &queue_mutex);
        }
        if (!queue_head && stop_requested) {
            pthread_mutex_unlock(&queue_mutex);
            break;
        }
        log_message_node *node = queue_head;
        queue_head = node->next;
        if (!queue_head) queue_tail = NULL;
        pthread_mutex_unlock(&queue_mutex);

        if (node && node->text) {
            if (!log.log_file) {
                // fallback to stdout
                printf("%s", node->text);
            } else {
                fprintf(log.log_file, "%s", node->text);
                fflush(log.log_file);
            }
        }
        free(node->text);
        free(node);
    }
    return NULL;
}

void set_log_file(const char *filename) {
    if (!filename) return;
    // Make a copy to avoid dangling pointer issues
    log.log_filename = strdup(filename);
}

// Config handlers

log_level_t get_log_level_from_str(const char *log_level_str) {
    if (strcmp(log_level_str, "DEBUG") == 0) return(DEBUG);
    else if (strcmp(log_level_str, "INFO") == 0) return(INFO);
    else if (strcmp(log_level_str, "WARNING") == 0) return(WARNING);
    else if (strcmp(log_level_str, "ERROR") == 0) return(ERROR);
    else if (strcmp(log_level_str, "FATAL") == 0) return(FATAL);
    else return DEBUG;
}

static void handle_log_level(const char *value) {
    set_log_level(get_log_level_from_str(value));
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
    set_log_level(get_log_level_from_str(level));
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
    // Build header text and enqueue for background logger
    time_t now = time(NULL);
    struct tm local_time;
    localtime_r(&now, &local_time);

    // Build args string first if needed
    char *args_str = NULL;
    size_t args_len = 0;
    if (argc > 0 && argv) {
        // Calculate total size needed for args
        for (int i = 0; i < argc; ++i) {
            args_len += snprintf(NULL, 0, " %s", argv[i] ? argv[i] : "(null)");
        }
        args_str = malloc(args_len + 1);
        if (!args_str) return;
        size_t offset = 0;
        for (int i = 0; i < argc; ++i) {
            const char *arg = argv[i] ? argv[i] : "(null)";
            offset += snprintf(args_str + offset, args_len - offset + 1, " %s", arg);
        }
    }

    // Build the full header
    int needed = snprintf(NULL, 0,
                          "==================== LOG START ====================\nTimestamp: %04d-%02d-%02d %02d:%02d:%02d\nProgram arguments (%d):%s\n===================================================\n",
                          local_time.tm_year + 1900, local_time.tm_mon + 1,
                          local_time.tm_mday, local_time.tm_hour,
                          local_time.tm_min, local_time.tm_sec, argc, args_str ? args_str : "");
    if (needed < 0) {
        free(args_str);
        return;
    }

    char *buf = malloc((size_t)needed + 1);
    if (!buf) {
        free(args_str);
        return;
    }

    if (snprintf(buf, (size_t)needed + 1,
                 "==================== LOG START ====================\nTimestamp: %04d-%02d-%02d %02d:%02d:%02d\nProgram arguments (%d):%s\n===================================================\n",
                 local_time.tm_year + 1900, local_time.tm_mon + 1,
                 local_time.tm_mday, local_time.tm_hour,
                 local_time.tm_min, local_time.tm_sec, argc, args_str ? args_str : "") < 0) {
        free(buf);
        free(args_str);
        return;
    }
    free(args_str);
    enqueue_text(buf);
}

void log_footer(void) {
    time_t now = time(NULL);
    struct tm local_time;
    localtime_r(&now, &local_time);

    int needed = snprintf(NULL, 0,
                          "===================== LOG END =====================\nTimestamp: %04d-%02d-%02d %02d:%02d:%02d\n===================================================\n",
                          local_time.tm_year + 1900, local_time.tm_mon + 1,
                          local_time.tm_mday, local_time.tm_hour,
                          local_time.tm_min, local_time.tm_sec);
    if (needed < 0) return;
    char *buf = malloc((size_t)needed + 1);
    if (!buf) return;
    snprintf(buf, (size_t)needed + 1,
             "===================== LOG END =====================\nTimestamp: %04d-%02d-%02d %02d:%02d:%02d\n===================================================\n",
             local_time.tm_year + 1900, local_time.tm_mon + 1,
             local_time.tm_mday, local_time.tm_hour,
             local_time.tm_min, local_time.tm_sec);
    enqueue_text(buf);
}

void init_logging(){
    if(!log.log_filename || log.log_filename[0] == '\0'){
        printf("[INFO] Logging on stdout!\n");
        log.log_file = stdout;
        // start background thread
        stop_requested = 0;
        if (pthread_create(&log_thread, NULL, log_worker, NULL) != 0) {
            fprintf(stderr, "Failed to create log worker thread\n");
            exit(EXIT_FAILURE);
        }
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

    // start background thread
    stop_requested = 0;
    if (pthread_create(&log_thread, NULL, log_worker, NULL) != 0) {
        fprintf(stderr, "Failed to create log worker thread\n");
        exit(EXIT_FAILURE);
    }

    log_empty_line();
    log_header(log.argc, log.argv);
}

void close_logging(){
    if (log.log_file) {
        log_footer();
    }

    // signal worker to stop after flushing queue
    pthread_mutex_lock(&queue_mutex);
    stop_requested = 1;
    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_mutex);

    pthread_join(log_thread, NULL);

    if(log.log_file && log.log_file != stdout){
        fclose(log.log_file);
        log.log_file = NULL;
        free(log.log_filename);
        log.log_filename = NULL;
    }
}

void log_todo(const char *func_name, const char *file_name, int line_number){
    logf_message(FATAL, "SYSTEM", "Function %s in %s:%d not yet implemented!", func_name, file_name, line_number);
    close_logging();
    exit(EXIT_FAILURE);
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
    char *buf = malloc(2);
    if (!buf) return;
    buf[0] = '\n';
    buf[1] = '\0';
    enqueue_text(buf);
}

bool will_log_level(log_level_t level){
    return level >= log.current_log_level;
}

void logf_message(log_level_t level, const char* module, const char* message, ...){
    if(level < log.current_log_level){
        return;
    }
    va_list args;
    va_start(args, message);
    va_list args_copy;
    va_copy(args_copy, args);
    int needed = vsnprintf(NULL, 0, message, args_copy);
    va_end(args_copy);
    if (needed < 0) {
        va_end(args);
        return;
    }
    char *msgbuf = malloc((size_t)needed + 1);
    if (!msgbuf) { va_end(args); return; }
    vsnprintf(msgbuf, (size_t)needed + 1, message, args);
    va_end(args);

    char *full = format_full_message(level, module, msgbuf);
    free(msgbuf);
    if (full) enqueue_text(full);
}

void log_message(log_level_t level, const char* module, const char* message){
    if(!module || !message){
        fprintf(stderr, "Error: Null parameter passed to log_message\n");
        return;
    }
    if(level < log.current_log_level) return;
    char *full = format_full_message(level, module, message);
    if (full) enqueue_text(full);
}

/*
    FOR JSON!
    fprintf(log.log_file, "{ \"timestamp\": %ld, \"level\": \"%s\", \"module\": \"%s\", \"message\": \"%s\" }\n",
            now, level_strings[level], module, text);
    */