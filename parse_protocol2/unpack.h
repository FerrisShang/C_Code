#ifndef __UNPACK_H__
#define __UNPACK_H__

#include "pool_format.h"
#include "pool_param.h"

struct parsed_item {
    struct param* param_type;
    int out_priority;
    int indent;
    uint8_t* data;
    char* title;
    int bit_width;
    char** lines;
    int line_num;
};

struct parsed_data {
    int items_num;
    struct parsed_item* items;
};

struct parsed_data unpack(uint8_t* data, int len);

#endif /* __UNPACK_H__ */
