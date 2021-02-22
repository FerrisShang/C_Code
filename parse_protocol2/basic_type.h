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
    BTYPE_TRUNCATED,
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
};

int output(int basic_type, uint8_t* data, int len, char* out_str, int* out_len);
bool is_basic_type(char* str);
const char* type_str(int basic_type);

#endif /* __BASIC_TYPE_H__ */
