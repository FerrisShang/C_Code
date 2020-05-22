#ifndef __HCI_CACHE_H__
#define __HCI_CACHE_H__

void hci_buf_in(uint8_t *data, int len);
uint8_t hci_buf_cnt(void);
uint8_t* hci_buf_get(uint8_t* len);

#endif /* __HCI_CACHE_H__ */
