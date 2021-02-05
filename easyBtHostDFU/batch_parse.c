#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "batch_parse.h"
#include "log.h"

#define SUC_FILENAME "dfu_success.csv"
#define MAX_STR_SIZE 1024
static dfu_items_t items_list;
static dfu_items_t items_success;
static char line_str[MAX_STR_SIZE];

static char* copy_hex(char* dst, char* src, int* max_len)
{
    int len = 0, flag = 0;
    if (!src) return src;
    while (isspace(*src))src++;
    while (len < *max_len && *src != '\0' && *src != ',') {
        if (isxdigit(*src)) {
            *dst = flag ? (*dst << 4) : 0;
            *dst += *src > '9' ? (*src | 0x20) + 10 - 'a' : *src - '0';
            if (flag) {
                len++;
                dst++;
            }
            flag = !flag;
        }
        src++;
    }
    *max_len = len;
    while (*src != ',' && *src != '\0')src++;
    if (*src) src++;
    return src;
}
static char* copy_string(char* dst, char* src, int max_len)
{
    if (!src) return src;
    while (isspace(*src))src++;
    while (isgraph(*src) && *src != ',') {
        *dst = *src;
        dst++, src++;
    }
    while (*src != ',' && *src != '\0')src++;
    if (*src) src++;
    *dst = '\0';
    return src;
}

static dfu_item_t* calloc_dfu_item(void)
{
    dfu_item_t* ret = calloc(1, sizeof(dfu_item_t));
    return ret;
}

static dfu_items_t* batch_parse(dfu_items_t* items, char* filename)
{
    FILE* fp;
    int line_num = 0;
    items->num = 0;
    fp = fopen(filename, "r");
    if (!fp) {
        log("[%s] not found, parse failed.", filename);
        return items;
    }
    dfu_item_t* item_add = calloc_dfu_item();
    while (!feof(fp)) {
        line_num++;
        line_str[0] = '\0'; fgets(line_str, MAX_STR_SIZE - 1, fp);
        char* t = line_str;
        while (!isgraph(*t) && *t != '\0') t++;
        if (!*t) {
            continue;
        }
        int l; char* p = line_str;
        p = copy_string(item_add->key, p, 32 - 1);
        if (!p) {
            log("Unexcepted data detected.(line:%d)\n", line_num);
            continue;
        }
        l = 6; p = copy_hex(item_add->old_addr, p, &l);
        if (!p) {
            log("Unexcepted data detected.(line:%d)\n", line_num);
            continue;
        }
        item_add->old_adv_len = 31; p = copy_hex(item_add->old_adv_data, p, &item_add->old_adv_len);
        if (!p) {
            log("Unexcepted data detected.(line:%d)\n", line_num);
            continue;
        }
        l = 6; p = copy_hex(item_add->new_addr, p, &l);
        if (!p) {
            log("Unexcepted data detected.(line:%d)\n", line_num);
            continue;
        }
        item_add->new_adv_len = 31; p = copy_hex(item_add->new_adv_data, p, &item_add->new_adv_len);
        if (!p) {
            log("Unexcepted data detected.(line:%d)\n", line_num);
            continue;
        }
        p = copy_string(item_add->filename, p, 128 - 1);
        if (strlen(item_add->filename) == 0) {
            log("Unexcepted data detected.(line:%d)\n", line_num);
            continue;
        }
        items->items[items->num] = item_add;
        items->num++;
        item_add = calloc_dfu_item();
    }
    fclose(fp);
    return items;
}

dfu_items_t* batch_parse_list(char* filename)
{
    chdir_to_file(filename);
    return batch_parse(&items_list, filename);
}

dfu_items_t* batch_parse_success(void)
{
    return batch_parse(&items_success, SUC_FILENAME);
}

dfu_items_t* batch_suc_add(dfu_item_t* item)
{
    printf("Add %d\n", 0);
    dfu_item_t* add = calloc_dfu_item();
    *add = *item;
    items_success.items[items_success.num++] = add;
    return &items_success;
}

dfu_items_t* batch_suc_save(void)
{
    int i;
    char buf[1024];
    char str_buf[4][128];
    FILE* fp = fopen(SUC_FILENAME, "w");
    for (i = 0; i < items_success.num; i++) {
        sprintf(buf, "%s,%s,%s,%s,%s,%s\n",
                items_success.items[i]->key,
                hex_to_str(str_buf[0], items_success.items[i]->old_addr, 6),
                hex_to_str(str_buf[1], items_success.items[i]->old_adv_data, items_success.items[i]->old_adv_len),
                hex_to_str(str_buf[2], items_success.items[i]->new_addr, 6),
                hex_to_str(str_buf[3], items_success.items[i]->new_adv_data, items_success.items[i]->new_adv_len),
                items_success.items[i]->filename);
        fputs(buf, fp);
    }
    fclose(fp);
    return &items_success;
}

