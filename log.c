//
// Created by slinky on 1/9/24.
//

#include "log.h"

#include <stdlib.h>
#include <string.h>

// log is oldest first, FIFO

int log_size;

char** log;

void init_log() {
    log_size = 0;
    log = malloc((sizeof (const char*)) * LOG_CAPACITY);
}

void push_log(const char* msg) {
    if (log_size == LOG_CAPACITY) {
        log[0] = NULL;
        for (int i = 1; i < LOG_CAPACITY; i++) {
            log[i - 1] = log[i];
            log[i] = NULL;
        }
        log_size--;
    }

    log[log_size] = malloc(strlen(msg));
    strcpy(log[log_size], msg);
    log_size++;
}

const char** get_log() {
    return log;
}

void free_log() {
    for (int i = 0; i < log_size; i++) {
        free(log[i]);
    }

    free(log);
}