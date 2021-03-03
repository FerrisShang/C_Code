#ifndef __UNPACK_H__
#define __UNPACK_H__

#include "pool_format.h"
#include "pool_param.h"

struct parsed_item {
    const struct param* param_type;
    int out_priority;
    int indent;
    const uint8_t* data;
    int bit_width;
    char* title;
    char** lines;
    int line_num;
};

struct parsed_data {
    struct parsed_item* item;
    struct parsed_data* next;
};

struct parsed_data* unpack(uint8_t* data, int len);
void unpack_free(struct parsed_data* data);

#endif /* __UNPACK_H__ */
