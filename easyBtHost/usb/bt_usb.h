#ifndef __BT_USB_H__
#define __BT_USB_H__

#include <stdint.h>

#ifdef __linux__
#include <libusb-1.0/libusb.h>
typedef libusb_device_handle USB_DEV_T;
#define BT_SNOOP_PATH   "./btsnoop_hci.log"
#else
#include "lusb0_usb.h"
typedef struct usb_dev_handle USB_DEV_T;
#define BT_SNOOP_PATH   ".\\btsnoop_hci.log"
#endif

// Device endpoint(s)
#define USB_EP_EVT_IN   (0x81)
#define USB_EP_CMD_OUT  (0x01 << 5)
#define USB_EP_ACL_IN   (0x82)
#define USB_EP_ACL_OUT  (0x02)

// Device vendor and product id.
#define MY_VID 0x0a5c
#define MY_PID 0x21ec
// Device configuration and interface id.
#define MY_CONFIG 1
#define MY_INTF   0

#define USB_LOG_NONE    0x00
#define USB_LOG_OUTPUT  0x01
#define USB_LOG_BTSNOOP 0x02
#define USB_LOG_ALL     (USB_LOG_OUTPUT | USB_LOG_BTSNOOP)


USB_DEV_T *usb_hci_init(int log_flag, void (*recv_cb)(uint8_t *data, int len));
void usb_hci_reinit(void (*cb)(void));
int usb_hci_send(uint8_t *data, int len);
int usb_hci_recv(uint8_t *data, int len, int endpoint);

#endif /* __BT_USB_H__ */
