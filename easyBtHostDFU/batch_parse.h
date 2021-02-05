#ifndef __BATCH_PARSE_H__
#define __BATCH_PARSE_H__
#include <stdint.h>

typedef struct {
    char key[32];
    uint8_t old_addr[6];
    uint8_t old_adv_data[32];
    int old_adv_len;
    uint8_t new_addr[6];
    uint8_t new_adv_data[32];
    int new_adv_len;
    char filename[128];
} dfu_item_t;

typedef struct {
    int num;
    dfu_item_t *items[4096];
} dfu_items_t;

dfu_items_t* batch_parse_list(char *filename);
dfu_items_t* batch_parse_success(void);
dfu_items_t* batch_suc_add(dfu_item_t *item);
dfu_items_t* batch_suc_save(void);

#endif /* __BATCH_PARSE_H__ */
