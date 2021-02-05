#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <time.h>
#include "arguments.h"
#include "utils.h"
#include "log.h"

struct arguments arguments = { ARG_FUNC_ERR };

static char* app_name;

void arg_set_app_name(char* name)
{
    app_name = name;
}
char* arg_get_app_name(void)
{
    return app_name;
}
void arg_parse(char* arg)
{
    struct arguments* args = &arguments;
    if (args->main_function == ARG_FUNC_ERR) {
        if (!strcasecmp(ARG_FUNC_SINGLE_STR, arg)) {
            args->main_function = ARG_FUNC_SINGLE;
        } else if (!strcasecmp(ARG_FUNC_BATCH_STR, arg)) {
            args->main_function = ARG_FUNC_BATCH;
        }
    } else if (args->main_function == ARG_FUNC_SINGLE || args->main_function == ARG_FUNC_BATCH) {
        args->filename = arg;
    } else {
        assert(0);
    }
}

void arg_dump(void)
{
    log("main_functin: %d\n", arguments.main_function);
    log("scan_time: %d\n", arguments.scan_time);
    log("is_check: %d\n", arguments.is_check);
    log("filename: %s\n", arguments.filename);
}

bool arg_check(void)
{
    if (!arguments.scan_time) {
        arguments.scan_time = 1;
    }
    return arguments.main_function != ARG_FUNC_ERR && arguments.filename;
}

