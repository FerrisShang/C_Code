#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "csv_read.h"
#include "pool_param.h"
int main(int argc, char* argv[])
{
    struct csv_data data;
    printf("%d\n", csv_read("param0.csv", &data));
    csv_dump();


    printf("%d\n", param_add("1name", 2, 6, 8, BTYPE_ENUM, "key_str", "range_str", "char* default_str", "char* output",
                             "char* description", 0x7FF, "POS1"));

    printf("%d\n", param_add("8name", 1, 2, 3, BTYPE_ENUM, "key_str", "range_str", "char* default_str", "char* output",
                             "char* description", 0x805, "POS2"));

    printf("%d\n", param_add("4name", 4, 5, 9, BTYPE_ENUM, "key_str", "range_str", "char* default_str", "char* output",
                             "char* description", 0x01, "POS3"));

    param_enum_add(param_get("4name"), "subkey1", 1248, "*output", "char *pos");
    param_enum_add(param_get("4name"), "subkey2", 2248, "*output", "char *pos");
    param_enum_add(param_get("4name"), "subkey3", 3248, "*output", "char *pos");
    param_enum_add(param_get("4name"), "subkey4", 4248, "*output", "char *pos");
    param_enum_add(param_get("4name"), "subkey5", 5248, "*output", "char *pos");



    pool_param_dump();
    return 0;
}
