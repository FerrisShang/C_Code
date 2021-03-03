#ifndef __BASIC_TYPE_H__
#define __BASIC_TYPE_H__
#include <stdint.h>
#include <stdbool.h>

#define BTYPE_ENUM_STR      "enum"
#define BTYPE_BITMAP_STR    "bitmap"
#define BTYPE_UNSIGNED_STR  "unsigned"
#define BTYPE_SIGNED_STR    "signed"
#define BTYPE_STREAM_STR    "stream"
#define BTYPE_HEX_STR       "hex"
#define BTYPE_ADDRESS_STR   "address"
#define BTYPE_T_0_625MS_STR "T0_625ms"
#define BTYPE_T_1_25MS_STR  "T1_25ms"
#define BTYPE_T_10MS_STR    "T10ms"

enum {
    BTYPE_SUCCESS,
    BTYPE_FAILED,
};

enum {
    BTYPE_ENUM,
    BTYPE_BITMAP,
    BTYPE_UNSIGNED,
    BTYPE_SIGNED,
    BTYPE_STREAM,
    BTYPE_HEX,
    BTYPE_ADDRESS,
    BTYPE_T_0_625MS,
    BTYPE_T_1_25MS,
    BTYPE_T_10MS,
    BTYPE_MAX,
    BTYPE_TRUNCATED = BTYPE_MAX,
    BTYPE_INVALID = -1,
};

int output_get(int basic_type, const uint8_t* data, int bit_len, char*** pp_out, int* out_num,
               const char* (*enum_str_cb)(int key, void* p), void* enum_p);
void output_free(char** pp_out, int out_num);
bool is_basic_type(char* str);
const char* type_str(int basic_type);
int type_idx(const char* type);

#endif /* __BASIC_TYPE_H__ */
