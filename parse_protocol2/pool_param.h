#ifndef __POOL_PARAM_H__
#define __POOL_PARAM_H__

#include "basic_type.h"
#include "csv_read.h"

enum {
    POOL_PARAM_SUCCESS,
    POOL_PARAM_REDEFINE,
    POOL_PARAM_ERR_FMT,
    POOL_PARAM_ERR_TYPE,
    POOL_PARAM_ERR_MEM,
};

struct enum_item {
    char* subkey;
    int value;
    char* output;
    char* pos; // param position
};

struct param {
    char* name;
    int bit_offset;
    int bit_width;
    int bit_length;
    int basic_type;
    int enum_num;
    struct enum_item* enum_items;
    char* key_str;
    char* range_str;
    char* default_str;
    char* output;
    char* description;
    int cfg_flag;
    int cfg_output_subkey;
    int cfg_inc_indent;
    int cfg_output_bit_is0;
    int cfg_param_can_longer;
    int cfg_length_ref;
    int cfg_unused;
    int cfg_out_priority;
    char* pos; // param position
};

int param_add(char* name, int bit_offset, int bit_width, int bit_length, int basic_type,
              char* key_str, char* range_str, char* default_str, char* output, char* description,
              int cfg_flag, char* pos);

int param_enum_add(struct param* p, char* subkey, int value, char* output, char* pos);
struct param* param_get(char* str);
void pool_param_dump(void);

#endif /* __POOL_PARAM_H__ */
