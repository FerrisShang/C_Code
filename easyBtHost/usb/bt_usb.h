#ifndef __BT_USB_H__
#define __BT_USB_H__

#include <stdint.h>
#include "lusb0_usb.h"

// Device vendor and product id.
#define MY_VID 0x0a5c
#define MY_PID 0x21ec

// Device configuration and interface id.
#define MY_CONFIG 1
#define MY_INTF   0

// Device endpoint(s)
#define USB_EP_EVT_IN   (USB_ENDPOINT_IN | 0x01)
#define USB_EP_CMD_OUT  (USB_TYPE_CLASS  | USB_ENDPOINT_OUT)
#define USB_EP_ACL_IN   (USB_ENDPOINT_IN | 0x02)
#define USB_EP_ACL_OUT  (0x02)

#define USB_LOG_NONE    0x00
#define USB_LOG_OUTPUT  0x01
#define USB_LOG_BTSNOOP 0x02
#define USB_LOG_ALL     (USB_LOG_OUTPUT | USB_LOG_BTSNOOP)

#define BT_SNOOP_PATH   ".\\x.log"

usb_dev_handle *usb_hci_init(int log_flag, void (*recv_cb)(uint8_t *data, int len));
void usb_hci_reinit(void (*cb)(void));
int usb_hci_send(uint8_t *data, int len);
int usb_hci_recv(uint8_t *data, int len, int endpoint);

#endif /* __BT_USB_H__ */
