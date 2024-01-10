//
// Created by slinky on 1/9/24.
//

#ifndef LOG_H
#define LOG_H

#define LOG_CAPACITY 25



void init_log();
void push_log();
const char** get_log();
void free_log();

#endif //LOG_H
