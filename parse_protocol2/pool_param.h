#ifndef __POOL_PARAM_H__
#define __POOL_PARAM_H__

#include "basic_type.h"
#include "csv_read.h"

enum {
    POOL_PARAM_SUCCESS,
    POOL_PARAM_REDEFINE,
    POOL_PARAM_KEY_MUST_INC,
    POOL_PARAM_ERR_FMT,
    POOL_PARAM_ERR_TYPE,
    POOL_PARAM_ERR_MEM,
};

enum {
    CFG_OUTPUT_SUBKEY_POS     = 0,
    CFG_OUTPUT_SUBKEY_MASK    = 0x01,
    CFG_INC_INDENT_POS        = 1,
    CFG_INC_INDENT_MASK       = 0x01,
    CFG_OUT_BIT_IS_0_POS      = 2,
    CFG_OUT_BIT_IS_0_MASK     = 0x01,
    CFG_PARAM_CAN_LONGER_POS  = 3,
    CFG_PARAM_CAN_LONGER_MASK = 0x01,
    CFG_LENTGH_REF_POS        = 4,
    CFG_LENTGH_REF_MASK       = 0x01,
    CFG_UNUSED_POS            = 5,
    CFG_UNUSED_MASK           = 0x01,
    CFG_OUTPUT_PRIORITY_POS   = 8,
    CFG_OUTPUT_PRIORITY_MASK  = 0x0F,
};
#define GET_BITS(cfg, data) ((data >> cfg##_POS)&cfg##_MASK)

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
    char* width_name;
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
    int cfg_out_priority; /* -8 ~ 7*/
    char* pos; // param position
    int flag_alias;
};

struct param* param_add(char* name, int bit_offset, int bit_width, int bit_length, char* width_name, int basic_type,
                        char* key_str, char* range_str, char* default_str, char* output, char* description,
                        int cfg_flag, char* pos);

int param_enum_add(struct param* p, char* subkey, int value, char* output, char* pos);
int param_alias(char* old_name, char* new_name, char* pos);
struct param* param_get(char* str);
int pool_param_iterate(int (*callback)(void* p, void* data), void* p);
void pool_param_dump(void);
void pool_param_free(void);

#endif /* __POOL_PARAM_H__ */
