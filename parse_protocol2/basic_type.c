#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "basic_type.h"
#include "utils.h"

#define b_calloc(s,n) util_calloc(s,n)
#define b_free(p) util_free(p)
#define debug printf

struct type_map {
    int type;
    const char* str;
};

const static struct type_map m_type_map[] = {
    { BTYPE_ENUM,      "enum"     },
    { BTYPE_BITMAP,    "bitmap"   },
    { BTYPE_UNSIGNED,  "unsigned" },
    { BTYPE_SIGNED,    "signed"   },
    { BTYPE_STREAM,    "stream"   },
    { BTYPE_HEX,       "hex"      },
    { BTYPE_ADDRESS,   "address"  },
    { BTYPE_T_0_625MS, "T0_625ms" },
    { BTYPE_T_1_25MS,  "T1_25ms"  },
    { BTYPE_T_10MS,    "T10ms"    },
};


bool is_basic_type(char* str)
{
    for (int i = 0; i < sizeof(m_type_map) / sizeof(m_type_map[0]); i++) {
        if (!strcmp(m_type_map[i].str, str)) {
            return true;
        }
    }
    return false;
}

int output_get(int basic_type, const uint8_t* data, int bit_len, char*** pp_out, int* out_num,
               const char* (*enum_str_cb)(int key, void* p), void* enum_p)
{
    if (basic_type > BTYPE_MAX) {
        return BTYPE_FAILED;
    }
    int32_t snum;
    uint32_t unum;
    char** p_out = NULL;
#define MAX_BUF 1000
    char buf[MAX_BUF + 1];
    if (basic_type == BTYPE_TRUNCATED) {
        *pp_out = (char**)b_calloc(sizeof(void*), 1);
        *out_num = 1;
        ** pp_out = strdup("[DATA TRUNCATED]");
        return BTYPE_SUCCESS;
    } else if (basic_type != BTYPE_BITMAP) { // NOTE:most of type only need 1 string in output list
        assert(!(bit_len & 0x7));
        *pp_out = (char**)b_calloc(sizeof(void*), 1);
        *out_num = 1;
        p_out = *pp_out;
    } else {
        // calloc memory in switch case
    }
    switch (basic_type) {
        case BTYPE_ENUM: {
            unum = hex2uint(data, bit_len / 8);
            const char* str = enum_str_cb(unum, enum_p);
            char num_str[32];
            uint2hexstr(num_str, unum, bit_len / 8);
            if (str) {
                snprintf(buf, MAX_BUF, "%s (%s)", str, num_str);
            } else {
                snprintf(buf, MAX_BUF, "Unknown (%s)", num_str);
            }
            p_out[0] = strdup(buf);
        } break;
        case BTYPE_UNSIGNED: {
            unum = hex2uint(data, bit_len / 8);
            snprintf(buf, MAX_BUF, "%d", unum);
            p_out[0] = strdup(buf);
        } break;
        case BTYPE_SIGNED: {
            snum = hex2int(data, bit_len / 8);
            snprintf(buf, MAX_BUF, "%d", snum);
            p_out[0] = strdup(buf);
        } break;
        case BTYPE_BITMAP: {
#if 1 // Just for NOW. TODO: support bitmap
            assert(!(bit_len & 0x7));
            *pp_out = (char**)b_calloc(sizeof(void*), 1);
            *out_num = 1;
            p_out = *pp_out;
#endif
        } // no break;
        case BTYPE_STREAM: {
            char buf[bit_len / 8 * 3], *p = buf;
            for (int i = 0; i < bit_len / 8; i++) {
                *p++ = (data[i] >> 4) >= 10 ? (data[i] >> 4) + 'A' - 10 : (data[i] >> 4) + '0';
                *p++ = (data[i] & 0xF) >= 10 ? (data[i] & 0xF) + 'A' - 10 : (data[i] & 0xF) + '0';
                *p++ = ' ';
            }
            *--p = '\0';
            p_out[0] = strdup(buf);
        } break;
        case BTYPE_HEX: {
            unum = hex2uint(data, bit_len / 8);
            uint2hexstr(buf, unum, bit_len / 8);
            p_out[0] = strdup(buf);
        } break;
        case BTYPE_ADDRESS: {
            char buf[bit_len / 8 * 3], *p = buf;
            for (int i = bit_len / 8 - 1; i >= 0; i--) {
                *p++ = (data[i] >> 4) >= 10 ? (data[i] >> 4) + 'A' - 10 : (data[i] >> 4) + '0';
                *p++ = (data[i] & 0xF) >= 10 ? (data[i] & 0xF) + 'A' - 10 : (data[i] & 0xF) + '0';
                *p++ = ':';
            }
            *--p = '\0';
            p_out[0] = strdup(buf);
        } break;
        case BTYPE_T_0_625MS: {
            unum = hex2uint(data, bit_len / 8);
            hex2ms(buf, unum, 0.625);
            p_out[0] = strdup(buf);
        } break;
        case BTYPE_T_1_25MS: {
            unum = hex2uint(data, bit_len / 8);
            hex2ms(buf, unum, 1.25);
            p_out[0] = strdup(buf);
        } break;
        case BTYPE_T_10MS: {
            unum = hex2uint(data, bit_len / 8);
            hex2ms(buf, unum, 10);
            p_out[0] = strdup(buf);
        } break;
    }
    return BTYPE_SUCCESS;
}
void output_free(char** pp_out, int out_num)
{
    for (int i = 0; i < out_num; i++) {
        free(pp_out[i]);
    }
    b_free(pp_out);
}

const char* type_str(int basic_type)
{
    if (basic_type >= BTYPE_MAX) {
        return NULL;
    }
    return m_type_map[basic_type].str;
}

int type_idx(const char* type)
{
    for (int i = 0; i < sizeof(m_type_map) / sizeof(m_type_map[0]); i++) {
        if (!strcmp(m_type_map[i].str, type)) {
            return i;
        }
    }
    return BTYPE_INVALID;
}
