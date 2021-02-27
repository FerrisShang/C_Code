#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "pool_param.h"
#include "pool_format.h"
#include "hashmap.h"
#include "utils.h"


#define MAX_FMT_LENGTH 32
#define MAX_FMT_LENGTH2 1024
#define p_calloc(s,n) calloc(s,n)
#define p_free(p) free(p)
#define pool_debug printf

struct pool_format {
    map_t map;
};

static struct pool_format m_formats;

struct format* format_add(char* name, char* pos)
{
    if (!m_formats.map) {
        m_formats.map = hashmap_new();
    }
    any_t arg;
    if (hashmap_get(m_formats.map, name, &arg) == MAP_OK) {
        return NULL;
    }
    struct format* p = p_calloc(sizeof(struct format), 1);
    assert(p);
    p->name = strdup(name);
    p->pos = strdup(pos);
    hashmap_put(m_formats.map, name, p);
    return p;
}

int format_item_add(struct format* p, char* remark, int key_code, char* param_list, char* pos)
{
    struct format_item* item = NULL;
    if (!p->items) {
        p->items = p_calloc(sizeof(struct format_item), MAX_FMT_LENGTH);
        assert(p->items);
    }
    if (p->format_num == MAX_FMT_LENGTH) {
        struct format_item* tmp = p->items;
        p->items = p_calloc(sizeof(struct format_item), MAX_FMT_LENGTH2);
        assert(p->items);
        memcpy(p->items, tmp, sizeof(struct format_item) * MAX_FMT_LENGTH);
        p_free(tmp);
    }
    if (p->format_num == MAX_FMT_LENGTH2) {
        return POOL_PARAM_ERR_MEM;
    }
    if (p->format_num && p->items[p->format_num - 1].key_code >= key_code) {
        return POOL_FORMAT_KEY_MUST_INC;
    }
    item = &p->items[p->format_num];
    item->remark = strdup(remark);
    item->key_code = key_code;
    item->pos = strdup(pos);
    item->params_num = 0;
    int max_num = strcnt(param_list, "|") + 1;
    item->params = p_calloc(sizeof(struct format_param), max_num);


    char* str = strdup(param_list);
    for (char* tok = strtok(str, "|"); tok && *tok; tok = strtok(NULL, "|")) {
        char* p = tok;
        while (*p == ' ') p++;
        char* type = p, *name;
        while (*p != 0 && *p != ' ') p++;
        if (*p == ' ') {
            *(char*)p = 0;
            name = p + 1;
        } else {
            name = type;
        }
        int len_type, len_name;
        type = strip(type, &len_type);
        name = strip(name, &len_name);
        if (type && len_type && name && len_name) {
            item->params[item->params_num].type = type;
            item->params[item->params_num].name = name;
            item->params_num++;
        }
    }
    // no free [str];
    p->format_num++;
    return POOL_FORMAT_SUCCESS;
}

struct format* format_get(char* str)
{
    if (m_formats.map) {
        any_t arg;
        if (hashmap_get(m_formats.map, str, &arg) == MAP_OK) {
            return (struct format*)arg;
        }
    }
    return NULL;
}

struct format_item* format_item_get(char* str, int key_code)
{
    struct format* p;
    if ((p = format_get(str)) != NULL) {
        int i;
        for (i = 0; i < p->format_num; i++) {
            struct format_item* item = &p->items[i];
            if (item->key_code == key_code) {
                return item;
            }
        }
    }
    return NULL;
}

static int hashmap_iterate_format_cb(any_t item, any_t data)
{
    struct format* p = data;
    int i, j;
    pool_debug("'%s': %d items pos:%s\n", p->name, p->format_num, p->pos);
    for (i = 0; i < p->format_num; i++) {
        pool_debug("\t'%s' %d pos:'%s'\t|", p->items[i].remark, p->items[i].key_code, p->items[i].pos);
        for (j = 0; j < p->items[i].params_num; j++) {
            pool_debug("%s.%s|", p->items[i].params[j].type, p->items[i].params[j].name);
        }
        pool_debug("\n");
    }
    return MAP_OK;
}

int pool_format_iterate(int (*callback)(void* p, void* data), void* p)
{
    if (m_formats.map) {
        return hashmap_iterate(m_formats.map, callback, p);
    } else {
        return POOL_FORMAT_ERR_MEM;
    }
}

void pool_format_dump(void)
{
    if (m_formats.map) {
        pool_debug("== FORMAT ==\n");
        hashmap_iterate(m_formats.map, hashmap_iterate_format_cb, NULL);
    } else {
        pool_debug("== FORMAT EMPTY ==\n");
    }
}

