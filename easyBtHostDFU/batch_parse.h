#ifndef __BATCH_PARSE_H__
#define __BATCH_PARSE_H__
#include <stdint.h>

#define MAX_ITEMS_NUM 65535
typedef struct {
    char key[32];
    uint8_t old_addr[7];
    uint8_t old_adv_data[32];
    int old_adv_len;
    uint8_t new_addr[7];
    uint8_t new_adv_data[32];
    int new_adv_len;
    char filename[128];
    uint8_t repeat;
    char is_valid;
} dfu_item_t;

typedef struct {
    int num;
    dfu_item_t *items[MAX_ITEMS_NUM];
} dfu_items_t;

dfu_items_t* batch_parse_list(char *filename);
dfu_items_t* batch_parse_success(dfu_items_t *item_list);
dfu_items_t* batch_suc_add(dfu_item_t* item, int update_file);
dfu_items_t* batch_suc_save(void);

#endif /* __BATCH_PARSE_H__ */
