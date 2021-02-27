#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "pool_param.h"
#include "hashmap.h"

#define MAX_ENUM_LENGTH 32
#define MAX_ENUM_LENGTH2 1024
#define p_calloc(s,n) calloc(s,n)
#define p_free(p) free(p)
#define pool_debug printf

struct pool_param {
    map_t map;
};

static struct pool_param m_params;

struct param* param_add(char* name, int bit_offset, int bit_width, int bit_length, char *width_name, int basic_type,
                        char* key_str, char* range_str, char* default_str, char* output, char* description,
                        int cfg_flag, char* pos)
{
    if (!m_params.map) {
        m_params.map = hashmap_new();
    }
    any_t arg;
    if (hashmap_get(m_params.map, name, &arg) == MAP_OK) {
        return NULL;
    }

    struct param* p = p_calloc(sizeof(struct param), 1);
    assert(p);
    p->name        = strdup(name);
    p->bit_offset  = bit_offset;
    p->bit_width   = bit_width;
    p->bit_length  = bit_length;
    p->width_name  = width_name?strdup(width_name):NULL;
    p->basic_type  = basic_type;
    p->key_str     = strdup(key_str);
    p->range_str   = strdup(range_str);
    p->default_str = strdup(default_str);
    p->output      = strdup(output);
    p->description = strdup(description);
    p->cfg_flag    = cfg_flag;
    p->cfg_output_subkey    = GET_BITS(CFG_OUTPUT_SUBKEY, cfg_flag);
    p->cfg_inc_indent       = GET_BITS(CFG_INC_INDENT, cfg_flag);
    p->cfg_output_bit_is0   = GET_BITS(CFG_OUT_BIT_IS_0, cfg_flag);
    p->cfg_param_can_longer = GET_BITS(CFG_PARAM_CAN_LONGER, cfg_flag);
    p->cfg_length_ref       = GET_BITS(CFG_LENTGH_REF, cfg_flag);
    p->cfg_unused           = GET_BITS(CFG_UNUSED, cfg_flag);
    int prio                = GET_BITS(CFG_OUTPUT_PRIORITY, cfg_flag); // range [-8, 7]
    p->cfg_out_priority     = prio > 7 ? (~prio) + 1 : prio;
    p->pos                  = strdup(pos);
    hashmap_put(m_params.map, name, p);
    return p;
}

int param_alias(char* old_name, char* new_name, char* pos)
{
    if (!m_params.map) {
        return POOL_PARAM_ERR_MEM;
    }
    struct param* p_old_param;
    if (hashmap_get(m_params.map, new_name, (any_t*)&p_old_param) == MAP_OK ||
            hashmap_get(m_params.map, old_name, (any_t*)&p_old_param) != MAP_OK) {
        return POOL_PARAM_REDEFINE;
    }
    struct param* p = p_calloc(sizeof(struct param), 1);
    assert(p);
    memcpy(p, p_old_param, sizeof(struct param));
    p->name        = strdup(new_name);
    p->pos = pos ? strdup(pos) : strdup(p_old_param->pos);
    hashmap_put(m_params.map, new_name, p);
    return POOL_PARAM_SUCCESS;
}

int param_enum_add(struct param* p, char* subkey, int value, char* output, char* pos)
{
    if (p->basic_type != BTYPE_ENUM && p->basic_type != BTYPE_BITMAP) {
        return POOL_PARAM_ERR_TYPE;
    }
    if (!p->enum_items) {
        p->enum_items = p_calloc(sizeof(struct enum_item), MAX_ENUM_LENGTH);
        assert(p->enum_items);
    }
    if (p->enum_num == MAX_ENUM_LENGTH) {
        struct enum_item* tmp = p->enum_items;
        p->enum_items = p_calloc(sizeof(struct enum_item), MAX_ENUM_LENGTH2);
        assert(p->enum_items);
        memcpy(p->enum_items, tmp, sizeof(struct enum_item) * MAX_ENUM_LENGTH);
        p_free(tmp);
    }
    if (p->enum_num == MAX_ENUM_LENGTH2) {
        return POOL_PARAM_ERR_MEM;
    }
    struct enum_item* e = &p->enum_items[p->enum_num];
    e->subkey = strdup(subkey);
    e->value = value;
    e->output = strlen(output) ? strdup(output) : NULL;
    e->pos = strdup(pos);
    if (p->enum_num && e->value <= p->enum_items[p->enum_num - 1].value) {
        return POOL_PARAM_KEY_MUST_INC;
    }
    p->enum_num++;
    return POOL_PARAM_SUCCESS;
}

struct param* param_get(char* str)
{
    if (m_params.map) {
        any_t arg;
        if (hashmap_get(m_params.map, str, &arg) == MAP_OK) {
            return (struct param*)arg;
        }
    }
    return NULL;
}

static int hashmap_iterate_cb(any_t item, any_t data)
{
    struct param* p = data;
    pool_debug("%s %s %d/%d/%d 0x%X %d %d %d %d %d %d %d %s\n", p->name, type_str(p->basic_type),
               p->bit_offset, p->bit_width, p->bit_length, p->cfg_flag, p->cfg_output_subkey,
               p->cfg_inc_indent, p->cfg_output_bit_is0, p->cfg_param_can_longer,
               p->cfg_length_ref, p->cfg_unused, p->cfg_out_priority, p->pos);
    if (p->basic_type == BTYPE_ENUM) {
        int i;
        for (i = 0; i < p->enum_num; i++) {
            pool_debug("\t%s:%d\n", p->enum_items[i].subkey, p->enum_items[i].value);
        }
    }
    return MAP_OK;
}

int pool_param_iterate(int (*callback)(void* p, void* data), void* p)
{
    if (m_params.map) {
        return hashmap_iterate(m_params.map, callback, p);
    } else {
        return POOL_PARAM_ERR_MEM;
    }
}

void pool_param_dump(void)
{
    if (m_params.map) {
        pool_debug("== PARAM ==\n");
        hashmap_iterate(m_params.map, hashmap_iterate_cb, NULL);
    } else {
        pool_debug("== FORMAT EMPTY ==\n");
    }
}
