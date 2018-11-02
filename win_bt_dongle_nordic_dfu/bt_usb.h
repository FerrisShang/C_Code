#ifndef __BT_USB_H__
#define __BT_USB_H__

#define __LINUX__

#ifdef __LINUX__
#include <libusb-1.0/libusb.h>
typedef libusb_device_handle USB_DEV_T;
#define USB_EP_EVT_IN   (0x81)
#define USB_EP_CMD_OUT  (0x01 << 5)
#define USB_EP_ACL_IN   (0x82)
#define USB_EP_ACL_OUT  (0x02)
#define BT_SNOOP_PATH   "~/fshang/btsnoop_hci.log"
#else
#include "lusb0_usb.h"
typedef usb_dev_handle USB_DEV_T;
// Device endpoint(s)
#define USB_EP_EVT_IN   (USB_ENDPOINT_IN | 0x01)
#define USB_EP_CMD_OUT  (USB_TYPE_CLASS  | USB_ENDPOINT_OUT)
#define USB_EP_ACL_IN   (USB_ENDPOINT_IN | 0x02)
#define USB_EP_ACL_OUT  (0x02)
#define BT_SNOOP_PATH   "D:\\x.log"
#endif

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

USB_DEV_T *hci_init(int log_flag, void (*recv_cb)(char *data, int len));
int hci_send(char *data, int len);
int hci_recv(char *data, int len, int endpoint);

#endif /* __BT_USB_H__ */
