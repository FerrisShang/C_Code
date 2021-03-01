#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "unpack.h"
#include "utils.h"

#define p_calloc(s,n) calloc(s,n)
#define p_free(p) free(p)
#define debug printf

#define MAX_DATA_SIZE 1024
static uint8_t m_data[MAX_DATA_SIZE];
#define MAX_UNPACK_ITEM_NUM 256
static struct parsed_item m_items[MAX_UNPACK_ITEM_NUM];
static int m_item_num;

enum {
    STATE_OK,
    STATE_NO_DATA,
    STATE_UNFINISHED,
};
static int m_state;

static void unpack_free(void)
{
    int i;
    for (i = 0; i < m_item_num; i++) {
        struct parsed_item* item = &m_items[i];
        p_free(item->title);
        output_free(item->lines, item->line_num);
        memset(item, 0, sizeof(struct parsed_item));
    }
    m_item_num = 0;
}
static char* enum_str_callback(int key, void* p)
{
    struct param* param = ((struct parsed_item*)p)->param_type;
    for (int i = 0; i < param->enum_num; i++) {
        if (param->enum_items[i].value == key) {
            if (param->enum_items[i].output && strlen(param->enum_items[i].output)) {
                return param->enum_items[i].output;
            } else {
                return param->enum_items[i].subkey;
            }
        }
    }
    return NULL;
}
static bool get_subkey_num(char* subkey_str, uint32_t* num)
{
    for (int i = m_item_num - 1; i >= 0; i--) {
        struct parsed_item* pitem = &m_items[i];
        if (!strcmp(pitem->param_type->name, subkey_str)) {
            *num = hex2uint(pitem->data, pitem->bit_width / 8);
            return true;
        }
    }
    return false;
}
static void unpack_data(uint8_t* data, int len, char* format_name, struct format_item* format_item, int indent)
{
    int i;
    int bit_offset = 0, bit_length = 0;
    for (i = 0; i < format_item->params_num; i++) {
        struct format_param* fmt_p = &format_item->params[i];
        struct param* param = param_get(fmt_p->type);
        assert(param);
        struct parsed_item* pitem = &m_items[m_item_num];

        // Adjust data & len & bit_length & bit_offset
        if (param->width_name || param->bit_offset < bit_offset || param->bit_length != bit_length ||
                param->bit_length <= 0 || param->bit_offset + param->bit_width > bit_length) {
            data += bit_length / 8;
            len -= bit_length / 8;
            bit_offset = bit_length = 0;
            if (len <= 0) {
                m_state = STATE_NO_DATA;
            }
        }
        // param type
        pitem->param_type = param;
        // out_priority
        if (param->basic_type != BTYPE_ENUM && param->basic_type != BTYPE_BITMAP &&
                param->key_str && strlen(param->key_str) && !param->cfg_output_subkey) {
            pitem->out_priority = -8;
        } else {
            pitem->out_priority = param->cfg_out_priority;
        }
        // indent
        pitem->indent = indent;
        // data
        pitem->data = data;
        // title
        pitem->title = strdup(fmt_p->name);
        // data_len
        if (m_state == STATE_OK) {
            if (param->width_name) { // length ref param
                assert(0); // TODO
            } else if (param->bit_length <= 0) {
                data += len + param->bit_length / 8;
                pitem->bit_width = len * 8 + param->bit_length;
                len  = -param->bit_length / 8;
            } else if (param->bit_width != param->bit_length) { // length bit shared
                bit_offset = param->bit_offset;
                bit_length = param->bit_length;
                pitem->bit_width = param->bit_width;
            } else { // normal bit length
                data += param->bit_length / 8;
                len -= param->bit_length / 8;
                pitem->bit_width = param->bit_width;
            }
            if (len < 0) {
                m_state = STATE_NO_DATA;
            } else {
                // lines
                if (param->bit_length != param->bit_width) {
                    uint64_t num = 0;
                    assert(param->bit_length < sizeof(uint32_t) * 8);
                    for (int i = param->bit_length / 8 - 1; i >= 0; i--) {
                        num <<= 8;
                        num += pitem->data[i];
                    }
                    num >>= param->bit_offset;
                    num &= ~(~(uint64_t)0 << param->bit_width);
                    output_get(param->basic_type, (uint8_t*)&num, (pitem->bit_width + 7) & ~0x7,
                               &pitem->lines, &pitem->line_num, enum_str_callback, pitem);
                } else {
                    output_get(param->basic_type, pitem->data, pitem->bit_width,
                               &pitem->lines, &pitem->line_num, enum_str_callback, pitem);
                }
            }
        }
        if (m_state == STATE_OK) {
            m_item_num++;
            // sub format
            if (param->basic_type != BTYPE_ENUM && param->basic_type != BTYPE_BITMAP &&
                    param->key_str && strlen(param->key_str)) {
                uint32_t enum_num;
                if (get_subkey_num(param->key_str, &enum_num)) {
                    struct format_item* fmt_item = format_item_get(fmt_p->name, enum_num);
                    if (fmt_item) {
                        unpack_data(pitem->data, pitem->bit_width / 8, fmt_p->name, fmt_item, indent + param->cfg_inc_indent);
                    } else {
                        struct parsed_item* pitem_err = &m_items[m_item_num];
                        pitem_err->param_type = NULL;
                        pitem_err->out_priority = 0;
                        pitem_err->indent = indent;
                        pitem_err->data = pitem->data;
                        pitem_err->bit_width = len * 8;
                        pitem_err->title = strdup("[Undecoded data]");
                        output_get(BTYPE_STREAM, pitem->data, pitem->bit_width, &pitem_err->lines, &pitem_err->line_num, NULL, NULL);
                        m_item_num++;
                    }
                } else {
                    debug("Tatal Error!!  Param '%s'(%s) not in format list @ %s\n", param->key_str, param->pos, format_item->pos);
                    assert(0);
                }
            }
        } else if (m_state == STATE_NO_DATA) {
            output_get(BTYPE_TRUNCATED, NULL, 0, &pitem->lines, &pitem->line_num, NULL, NULL);
            m_item_num++;
            break;
        } else {
            assert(0);
        }
    }
    if (len > 0) { // Some data not in format items
        struct parsed_item* pitem = &m_items[m_item_num];
        pitem->param_type = NULL;
        struct param* p = param_get(format_name);
        pitem->out_priority = p && p->cfg_param_can_longer ? 0 : -8;
        pitem->indent = indent;
        pitem->data = data;
        pitem->bit_width = len * 8;
        pitem->title = strdup("[Undecoded data]");
        output_get(BTYPE_STREAM, data, pitem->bit_width, &pitem->lines, &pitem->line_num, NULL, NULL);
        m_item_num++;
    }
}

struct parsed_data unpack(uint8_t* data, int len)
{
    unpack_free();

    m_state = STATE_OK;
#define PROTO_ROOT        "PROTO_ALL"
    assert(len < MAX_DATA_SIZE);
    memcpy(m_data, data, len);
    unpack_data(m_data, len, PROTO_ROOT, format_item_get(PROTO_ROOT, 0), 0);
    struct parsed_data ret = {m_item_num, m_items};
    return ret;
}

