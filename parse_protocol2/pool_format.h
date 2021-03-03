#ifndef __POOL_FORMAT_H__
#define __POOL_FORMAT_H__

#include "basic_type.h"
#include "csv_read.h"

enum {
    POOL_FORMAT_SUCCESS,
    POOL_FORMAT_REDEFINE,
    POOL_FORMAT_KEY_MUST_INC,
    POOL_FORMAT_ERR_FMT,
    POOL_FORMAT_ERR_TYPE,
    POOL_FORMAT_ERR_MEM,
};

struct format_param {
    char* type;
    char* name;
};

struct format_item {
    char* remark;
    int key_code;
    int params_num;
    struct format_param* params;
    char* pos; // position
};

struct format {
    char* name;
    int format_num;
    struct format_item* items;
    char* pos; // position
};

struct format* format_add(char* name, char* pos);
int format_item_add(struct format* p, char* remark, int key_code, char* param_list, char* pos);
struct format* format_get(char* str);
struct format_item* format_item_get(char* str, int key_code);
void pool_format_free(void);
int pool_format_iterate(int (*callback)(void* p, void* data), void* p);
void pool_format_dump(void);

#endif /* __POOL_FORMAT_H__ */
