#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "unpack.h"
#include "utils.h"

#define p_calloc(s,n) util_calloc(s,n)
#define p_free(p) util_free(p)
#define debug printf

enum {
    STATE_OK,
    STATE_NO_DATA,
    STATE_UNFINISHED,
};

struct unpack_env {
    int state;
    uint8_t* data;
    int data_len;
    struct parsed_data list_head;
};

static void free_data_list(struct parsed_data* data)
{
    if (data->next) {
        free_data_list(data->next);
    }
    if (data) {
        struct parsed_item* p = data->item;
        free(p->title);
        output_free(p->lines, p->line_num);
        p_free(p);
        p_free(data);
    }
}
void unpack_free(struct parsed_data* data)
{
#define container_of(ptr, type, member) ((type *)((char *)ptr - ((size_t) &((type *)0)->member)))
    struct unpack_env* env = container_of(data, struct unpack_env, list_head);
    free_data_list(env->list_head.next);
    p_free(env->data);
    p_free(env);
}
static const char* enum_str_callback(int key, void* p)
{
    const struct param* param = ((struct parsed_item*)p)->param_type;
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
static bool get_subkey_num(char* subkey_str, uint32_t* num, struct parsed_data* p)
{
    if (p->next && get_subkey_num(subkey_str, num, p->next)) {
        return true;
    } else {
        struct parsed_item* pitem = p->item;
        if (!strcmp(pitem->param_type->name, subkey_str)) {
            *num = hex2uint(pitem->data, pitem->bit_width / 8);
            return true;
        }
    }
    return false;
}
static struct parsed_data* unpack_data(const uint8_t* data, int len, char* format_name, struct format_item* format_item,
                                       int indent, struct unpack_env* env, struct parsed_data* p_last_data)
{
    int i;
    int bit_offset = 0, bit_length = 0;
    struct parsed_data* p_cur_data = p_last_data;
    for (i = 0; i < format_item->params_num; i++) {
        struct format_param* fmt_p = &format_item->params[i];
        struct param* param = param_get(fmt_p->type);
        assert(param);
        p_cur_data = p_cur_data->next = (struct parsed_data*)p_calloc(sizeof(struct parsed_data), 1);
        assert(p_cur_data);
        struct parsed_item* pitem = p_cur_data->item = (struct parsed_item*)p_calloc(sizeof(struct parsed_item), 1);
        assert(pitem);

        // Adjust data & len & bit_length & bit_offset
        if (param->width_name || param->bit_offset < bit_offset || param->bit_length != bit_length ||
                param->bit_length <= 0 || param->bit_offset + param->bit_width > bit_length) {
            data += bit_length / 8;
            len -= bit_length / 8;
            bit_offset = bit_length = 0;
            if (len <= 0) {
                env->state = STATE_NO_DATA;
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
        if (env->state == STATE_OK) {
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
                env->state = STATE_NO_DATA;
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
        if (env->state == STATE_OK) {
            // sub format
            if (param->basic_type != BTYPE_ENUM && param->basic_type != BTYPE_BITMAP &&
                    param->key_str && strlen(param->key_str)) {
                uint32_t enum_num;
                if (get_subkey_num(param->key_str, &enum_num, env->list_head.next)) {
                    struct format_item* fmt_item = format_item_get(fmt_p->name, enum_num);
                    if (fmt_item) {
                        p_cur_data = unpack_data(pitem->data, pitem->bit_width / 8, fmt_p->name, fmt_item, indent + param->cfg_inc_indent, env,
                                                 p_cur_data);
                    } else {
                        struct parsed_data* next_data = p_cur_data->next = (struct parsed_data*)p_calloc(sizeof(struct parsed_data), 1);
                        assert(next_data);
                        struct parsed_item* pitem_err = next_data->item = (struct parsed_item*)p_calloc(sizeof(struct parsed_item), 1);
                        assert(pitem_err);
                        pitem_err->param_type = NULL;
                        pitem_err->out_priority = 0;
                        pitem_err->indent = indent;
                        pitem_err->data = pitem->data;
                        pitem_err->bit_width = len * 8;
                        pitem_err->title = strdup("[Undecoded data]");
                        output_get(BTYPE_STREAM, pitem->data, pitem->bit_width, &pitem_err->lines, &pitem_err->line_num, NULL, NULL);
                        p_cur_data = next_data;
                    }
                } else {
                    debug("Tatal Error!!  Param '%s'(%s) not in format list @ %s\n", param->key_str, param->pos, format_item->pos);
                    assert(0);
                }
            }
        } else if (env->state == STATE_NO_DATA) {
            output_get(BTYPE_TRUNCATED, NULL, 0, &pitem->lines, &pitem->line_num, NULL, NULL);
            break;
        } else {
            assert(0);
        }
    }
    if (len > 0) { // Some data not in format items
        struct parsed_data* next_data = p_cur_data->next = (struct parsed_data*)p_calloc(sizeof(struct parsed_data), 1);
        assert(next_data);
        struct parsed_item* pitem = next_data->item = (struct parsed_item*)p_calloc(sizeof(struct parsed_item), 1);
        assert(pitem);
        pitem->param_type = NULL;
        struct param* p = param_get(format_name);
        pitem->out_priority = p && p->cfg_param_can_longer ? 0 : -8;
        pitem->indent = indent;
        pitem->data = data;
        pitem->bit_width = len * 8;
        pitem->title = strdup("[Undecoded data]");
        output_get(BTYPE_STREAM, data, pitem->bit_width, &pitem->lines, &pitem->line_num, NULL, NULL);
        p_cur_data = next_data;
    }
    return p_cur_data;
}

struct parsed_data* unpack(uint8_t* data, int len)
{
#define PROTO_ROOT        "PROTO_ALL"
    struct unpack_env* env = (struct unpack_env*)p_calloc(sizeof(struct unpack_env), 1);
    env->state = STATE_OK;
    env->data_len = len;
    env->data = (uint8_t*)p_calloc(len, 1);
    memcpy(env->data, data, len);
    unpack_data(env->data, len, PROTO_ROOT, format_item_get(PROTO_ROOT, 0), 0, env, &env->list_head);
    return &env->list_head;
}

