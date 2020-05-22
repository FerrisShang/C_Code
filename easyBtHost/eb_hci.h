#ifndef __EB_HCI_H__
#define __EB_HCI_H__

#include "eb_config.h"
#include "easyBle.h"

void eb_hci_init(void);
void eb_hci_handler(uint8_t *data, uint16_t len);

bool em_hci_encrypt(uint8_t *plaintext, uint8_t *key, void (*encrypt_cb)(uint8_t *encrypted));


#endif /* __EB_HCI_H__ */
