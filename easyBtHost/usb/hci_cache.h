#ifndef __HCI_CACHE_H__
#define __HCI_CACHE_H__

void hci_buf_in(uint8_t *data, uint32_t len);
uint32_t hci_buf_cnt(void);
uint8_t* hci_buf_get(uint32_t* len);
int hci_buf_is_full(void);
void hci_buf_clear(void);

#endif /* __HCI_CACHE_H__ */
