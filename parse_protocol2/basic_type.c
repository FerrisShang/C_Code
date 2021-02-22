#include <string.h>
#include "basic_type.h"

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
    int i;
    for (i = 0; i < sizeof(m_type_map) / sizeof(m_type_map[0]); i++) {
        if (!strcmp(m_type_map[i].str, str)) {
            return true;
        }
    }
    return false;
}

int output(int basic_type, uint8_t* data, int len, char* out_str, int* out_len)
{
    if (basic_type >= BTYPE_MAX) {
        return BTYPE_TRUNCATED;
    }
    return BTYPE_SUCCESS;
}

const char* type_str(int basic_type)
{
    if (basic_type >= BTYPE_MAX) {
        return NULL;
    }
    return m_type_map[basic_type].str;
}
