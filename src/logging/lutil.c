#include "lutil.h"

#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#define MAX_MODULES 10
#define MODULE_NAME_LENGTH 20

//Config
void load_config(const char* config_file){
    FILE* file = fopen(config_file, "r");
    if(!file){
        fprintf(stderr, "Failed to open config file: %s\n", config_file);
        return;
    }
    
    bool clear_file = false;

    char line[128];
    while(fgets(line, sizeof(line), file)){
        if (strncmp(line, "log_level=", 10) == 0){
            char* level = line + 10;
            level[strcspn(level, "\n")] = '\0';
            if (strcmp(level, "DEBUG") == 0) set_log_level(DEBUG);
            else if (strcmp(level, "INFO") == 0) set_log_level(INFO);
            else if (strcmp(level, "WARNING") == 0) set_log_level(WARNING);
            else if (strcmp(level, "ERROR") == 0) set_log_level(ERROR);
        } else if (strncmp(line, "log_file=", 9) == 0) {
            char* filename = line + 9;
            filename[strcspn(filename, "\n")] = '\0';
            init_logging(filename, clear_file);
        } else if (strncmp(line, "clear_file=", 11) == 0){
            char* val = line + 11;
            val[strcspn(val, "\n")] = '\0';
            if (strcmp(val, "false") == 0) clear_file = false;
            else if (strcmp(val, "true") == 0) clear_file = true;
        }
    }

    fclose(file);
}

//Log Level
static log_level_t current_log_level = INFO;

void set_log_level(log_level_t level){
    current_log_level = level;
}

void set_log_level_from_string(const char* level){
    if (strcmp(level , "DEBUG") == 0) set_log_level(DEBUG); 
    else if (strcmp(level , "INFO") == 0) set_log_level(INFO); 
    else if (strcmp(level , "WARNING") == 0) set_log_level(WARNING); 
    else if (strcmp(level , "ERROR") == 0) set_log_level(ERROR); 
    else if (strcmp(level , "FATAL") == 0) set_log_level(FATAL); 
}

//FILE
static FILE* log_file = NULL;

void init_logging(const char* filename, bool clear_file){
    if(!filename){
        log_file = stdout;
        return;
    }
    if (clear_file) log_file = fopen(filename, "w");
    else log_file = fopen(filename, "a");
    if(!log_file){
        fprintf(stderr, "Failed to open log file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
}

void close_logging(){
    if(log_file && log_file != stdout){
        fclose(log_file);
        log_file = NULL;
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
    if (!log_file) {
        fprintf(stderr, "Logging not initialized.\n");
        return;
    }
    fprintf(log_file, "\n");
    fflush(log_file);
}

void log_message_header(){
    return;
}

void logf_message(log_level_t level, const char* module, const char* message, ...){
    va_list args;
    va_start(args, message);
    
    const char* level_strings[] = { "DEBUG", "INFO", "WARNING", "ERROR", "FATAL" };
    time_t now = time(NULL);
    struct tm* local_time = localtime(&now);

    fprintf(log_file, "%04d-%02d-%02d %02d:%02d:%02d [%s][%s] ",
            local_time->tm_year + 1900, local_time->tm_mon+1,
            local_time->tm_mday, local_time->tm_hour, local_time->tm_min,
            local_time->tm_sec, level_strings[level], module);
    vfprintf(log_file, message, args);
    fprintf(log_file, "\n");
    fflush(log_file);
    va_end(args);
}

void log_message(log_level_t level, const char* module, const char* message){
    if(!module || !message){
        fprintf(stderr, "Error: Null parameter passed to log_message\n");
        return;
    }

    if(level < current_log_level){
        return;
    }

    if (!log_file) {
        fprintf(stderr, "Logging not initialized.\n");
        return;
    }

    const char* level_strings[] = { "DEBUG", "INFO", "WARNING", "ERROR", "FATAL" };
    time_t now = time(NULL);
    struct tm* local_time = localtime(&now);

    fprintf(log_file, "%04d-%02d-%02d %02d:%02d:%02d [%s][%s] %s\n",
            local_time->tm_year + 1900, local_time->tm_mon+1,
            local_time->tm_mday, local_time->tm_hour, local_time->tm_min,
            local_time->tm_sec, level_strings[level], module, message);
    
    fflush(log_file);
}

/*
    FOR JSON!
    fprintf(log_file, "{ \"timestamp\": %ld, \"level\": \"%s\", \"module\": \"%s\", \"message\": \"%s\" }\n",
            now, level_strings[level], module, text);
    */