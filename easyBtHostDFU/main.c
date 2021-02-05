#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include <getopt.h>
#include <assert.h>
#include <sys/time.h>
#include "arguments.h"
#include "utils.h"

/* Program documentation. */
static char doc[] = \
                    "Usage: DFU [OPTION...] [single|batch|] ...\n"
                    "Update a single frameware.\n"
                    "#   DFU single filename[.zip]\n"
                    "Batch update frameware.\n"
                    "#   DFU batch filename[.csv]\n"
                    "\n"
                    "  -s, --scan[=tiem_s]   Set time of scan duration after dfu finished. default: 1s\n"
                    "  -c, --check           Check device which dfu finished by batch device list instead of updating\n"
                    "  -h, --help            give this help list\n"
                    "\n"
                    " Note:\n"
                    "   Batch filelist format per line:\n"
                    "     KEY, ADDR, ADV Data, ADDR(after DFU), ADV data(after DFU), Frameware_file_name.zip\n"
                    "\n";

/* The options we understand. */
static struct option long_options[] = {
    {"check",  no_argument,       0, 'c' },
    {"scan",   required_argument, 0, 's' },
    {"help",   no_argument,       0, 'h' },
    { 0 }
};
static const char optstring[] = "cs:h";
static void args_usage(void)
{
    log("%s", doc);
}
static void args_parse(int argc, char** argv, struct arguments* args)
{
    int c;
    while (1) {
        int option_index = 0;
        c = getopt_long(argc, argv, optstring, long_options, &option_index);
        if (c == -1) {
            break;
        }
        switch (c) {
            case 'c':
                args->is_check = 1;
                break;
            case 's':
                if (optarg && is_valid_num(optarg)) {
                    args->scan_time = str2u32(optarg);
                }
                break;
            case 'h':
            default:
                args_usage();
                exit(-1);
        }
    }
    if (optind < argc) {
        while (optind < argc) {
            arg_parse(argv[optind++]);
        }
    }
}

int main (int argc, char** argv)
{
    arg_set_app_name(argv[0]);
    args_parse(argc, argv, &arguments);
    if (!arg_check()) {
        args_usage();
        exit(-1);
    }
    //arg_dump();
    if (arguments.main_function == ARG_FUNC_SINGLE) {
        extern void dfu_single(char* filename);
        dfu_single(arguments.filename);
    } else if (arguments.main_function == ARG_FUNC_BATCH) {
        extern void dfu_batch(char* filename, int scan_time, int is_check);
        //dfu_batch(arguments.filename, arguments.scan_time, arguments.is_check);
    } else {
        assert(0);
    }
    exit (0);
}
