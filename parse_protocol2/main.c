#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "csv_read.h"
#include "pool_param.h"
#include "pool_format.h"
#include "parse_init.h"
#include "unpack.h"

int main(int argc, char* argv[])
{
    parse_init();
    //pool_format_dump();
    //pool_param_dump();
    unpack((uint8_t*)"\x01\x03\x0c\x00", 4);
    return 0;
}
