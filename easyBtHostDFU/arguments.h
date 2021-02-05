#ifndef __ARGUMENTS_H__
#define __ARGUMENTS_H__

#include <stdint.h>
#include "stdbool.h"
#define log printf

enum {
    ARG_FUNC_SINGLE,
    ARG_FUNC_BATCH,
    ARG_FUNC_ERR = -1,
};

#define ARG_FUNC_SINGLE_STR  "single"
#define ARG_FUNC_BATCH_STR "batch"

/* Used by main to communicate with parse_opt. */
struct arguments {
    int main_function;
    int scan_time;
    int is_check;
    char* filename;
};

extern struct arguments arguments;

void arg_set_app_name(char* name);
char* arg_get_app_name(void);
void arg_parse(char* arg);
void arg_dump(void);
bool arg_check(void);

#endif /* __ARGUMENTS_H__ */
