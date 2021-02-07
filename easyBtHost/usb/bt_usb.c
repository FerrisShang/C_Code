// sourceforge.net/projects/libusb-win32
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include "bt_usb.h"
#include "btsnoop_rec.h"

#define dump(d, l) do{int i;for(i=0;i<l;i++)printf("%02X ", (unsigned char)d[i]);printf("\n");fflush(stdout);}while(0)
#define TRAN_TOUT (500)
#define LOG_IN    (1)
#define LOG_OUT   (0)

static int _log_flag, _exit_flag;
static FILE* bt_snoop;
static void (*_recv_cb)(uint8_t* data, int len);
static pthread_mutex_t log_lock, callback_lock;
static USB_DEV_T* usb_dev = NULL;
static pthread_t th1, th2;

static void log_data(uint8_t* data, int len, int dir)
{
    pthread_mutex_lock(&log_lock);
    if ((_log_flag & USB_LOG_OUTPUT) && dir == LOG_IN) {
        printf(">>> ");
        dump(data, len);
    } else if ((_log_flag & USB_LOG_OUTPUT) && dir == LOG_OUT) {
        printf("<<< ");
        dump(data, len);
    }
    if (_log_flag & USB_LOG_BTSNOOP) {
        record_btsnoop(bt_snoop, data, len, dir);
    }
    pthread_mutex_unlock(&log_lock);
}

static void* hci_read_th(void* p)
{
    int ep = *(int*)p;
    uint8_t buf[1024];
    while (1) {
        int res = usb_hci_recv(buf, 1024, ep);
        if(_exit_flag){
            break;
        }
        if (res > 0){
            pthread_mutex_lock(&callback_lock);
            _recv_cb(buf, res);
            pthread_mutex_unlock(&callback_lock);
        } else if(res == -EIO){
            pthread_mutex_lock(&callback_lock);
            if(!_exit_flag){ _recv_cb((uint8_t*)"\x04\x10\x00\x00", 4); }
            _exit_flag = true;
            pthread_mutex_unlock(&callback_lock);
            break;
        } else {
            usleep(1000);
        }
    }
    return NULL;
}
#ifdef __linux__

static USB_DEV_T* open_dev(libusb_context* ctx)
{
    libusb_device** list;
    USB_DEV_T* dev_handle = NULL;
    libusb_get_device_list(ctx, &list);
    libusb_device* dev;
    int i = 0;
    while ((dev = list[i++]) != NULL) {
        struct libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(dev, &desc);
        if (r < 0) {
            continue;
        }
        //printf("%04x %04x\n", desc.idVendor, desc.idProduct);
        if (desc.idVendor == MY_VID && desc.idProduct == MY_PID) {
            libusb_open(dev, &dev_handle);
            if (dev_handle) {
                break;
            }
        }
    }
    libusb_free_device_list(list, 0);
    if (dev_handle) {
        libusb_reset_device(dev_handle);
    }
    return dev_handle;
}

USB_DEV_T* get_usb_dev(void)
{
    if (usb_dev) return usb_dev;
    libusb_context* ctx;
    int r = libusb_init(&ctx);
    if (r < 0) {
        fprintf(stderr, "failed to initialise libusb\n");
        exit(1);
    }
    usb_dev = open_dev(ctx);
    if (!usb_dev) {
        fprintf(stderr, "Could not find/open device\n");
        goto out;
    }
    libusb_detach_kernel_driver(usb_dev, 0);
    r = libusb_set_configuration(usb_dev, MY_CONFIG);
    if (r < 0) {
        printf("error setting config #%d: %s\n", MY_CONFIG, libusb_strerror(r));
        goto out;
    }
    r = libusb_claim_interface(usb_dev, 0);
    if (r < 0) {
        fprintf(stderr, "usb_claim_interface error %d\n", r);
        goto out;
    }
    //printf("claimed interface\n");
    return usb_dev;
out:
    libusb_close(usb_dev);
    libusb_exit(ctx);
    exit(1);
}

int usb_hci_send(uint8_t* data, int len)
{
    int ret = -1;
    USB_DEV_T* dev = get_usb_dev();
    assert(len >= 1 && (data[0] == 0x01 || data[0] == 0x02));
    if (data[0] == 0x01) { //cmd
        ret = libusb_control_transfer(dev, USB_EP_CMD_OUT, 0, 0, 0, data + 1, len - 1, TRAN_TOUT);
    } else if (data[0] == 0x02) { //acl
        int recv_len;
        ret = libusb_bulk_transfer(dev, USB_EP_ACL_OUT, data + 1, len - 1, &recv_len, TRAN_TOUT);
    }
    if (ret >= 0) log_data(data, len, LOG_OUT);
    return ret;
}

int usb_hci_recv(uint8_t* data, int len, int endpoint)
{
    assert(len > 0 && (endpoint == USB_EP_EVT_IN || endpoint == USB_EP_ACL_IN));
    int recv_len = -1;
    USB_DEV_T* dev = get_usb_dev();
    libusb_bulk_transfer(dev, endpoint, data + 1, len - 1, &recv_len, TRAN_TOUT);
    if (recv_len > 0) {
        data[0] = endpoint == USB_EP_EVT_IN ? 0x04 : 0x02;
        log_data(data, recv_len + 1, LOG_IN);
        return recv_len + 1;
    }
    return recv_len;
}
#else
static USB_DEV_T* open_dev(void)
{
    struct usb_bus* bus;
    struct usb_device* dev;
    for (bus = usb_get_busses(); bus; bus = bus->next) {
        for (dev = bus->devices; dev; dev = dev->next) {
            if (dev->descriptor.idVendor == MY_VID && dev->descriptor.idProduct == MY_PID) {
                USB_DEV_T* usb_dev = usb_open(dev);
                if (usb_set_configuration(usb_dev, MY_CONFIG) < 0) {
                    //printf("error setting config #%d: %s\n", MY_CONFIG, usb_strerror());
                    usb_close(usb_dev);
                    continue;
                }
                if (usb_claim_interface(usb_dev, 0) < 0) {
                    //printf("error claiming interface #%d:\n%s\n", MY_INTF, usb_strerror());
                    usb_close(usb_dev);
                    continue;
                }
                //usb_reset(usb_dev);
                return usb_dev;
            }
        }
    }
    return NULL;
}

static USB_DEV_T* get_usb_dev(void)
{
    if (usb_dev) return usb_dev;
    usb_init(); /* initialize the library */
    usb_find_busses(); /* find all busses */
    usb_find_devices(); /* find all connected devices */
    if (!(usb_dev = open_dev())) {
        printf("error opening device: \n%s\n", usb_strerror());
        exit(-1);
    }
    return usb_dev;
}

int usb_hci_send(uint8_t* data, int len)
{
    int ret = 0;
    assert(len >= 1 && (data[0] == 0x01 || data[0] == 0x02));
    if (!usb_dev) return -1;
    if (data[0] == 0x01) { //cmd
        ret = usb_control_msg(usb_dev, USB_EP_CMD_OUT, 0, 0, 0, (char*)(data + 1), len - 1, TRAN_TOUT);
    } else if (data[0] == 0x02) { //acl
        ret = usb_bulk_write(usb_dev, USB_EP_ACL_OUT, (char*)(data + 1), len - 1, TRAN_TOUT);
    }
    if (ret > 0) log_data(data, len, LOG_OUT);
    return ret;
}

int usb_hci_recv(uint8_t* data, int len, int endpoint)
{
    assert(len > 0 && (endpoint == USB_EP_EVT_IN || endpoint == USB_EP_ACL_IN));
    if (!usb_dev) return -1;
    int ret = usb_bulk_read(get_usb_dev(), endpoint, (char*)(data + 1), len - 1, TRAN_TOUT);
    if (ret > 0) {
        data[0] = endpoint == USB_EP_EVT_IN ? 0x04 : 0x02;
        log_data(data, ret + 1, LOG_IN);
        return ret + 1;
    } else {
        return ret;
    }
}
#endif


USB_DEV_T* usb_hci_init(int log_flag, void (*recv_cb)(uint8_t* data, int len))
{
    _exit_flag = false;
    USB_DEV_T* ret = get_usb_dev();
    _log_flag = log_flag;
    _recv_cb = recv_cb;
    pthread_mutex_init(&log_lock, NULL);
    pthread_mutex_init(&callback_lock, NULL);
    if (_log_flag & USB_LOG_BTSNOOP) {
        bt_snoop = create_btsnoop_rec(BT_SNOOP_PATH);
    }
    if (_recv_cb) {
        static int th1_ep = (USB_EP_EVT_IN), th2_ep = (USB_EP_ACL_IN);
        pthread_create(&th1, 0, hci_read_th, &th1_ep);
        pthread_create(&th2, 0, hci_read_th, &th2_ep);
    }
    return ret;
}

void usb_hci_deinit()
{
    _exit_flag = true;
    pthread_join(th1, NULL);
    pthread_join(th2, NULL);
    if(!usb_dev){
        return;
    }
#ifdef __linux__
    libusb_reset_device(usb_dev);
    libusb_close(usb_dev);
#else
    //usb_reset(usb_dev);
    usb_close(usb_dev);
#endif
    bt_snoop = NULL;
    usb_dev = NULL;
    usleep(500000);
}

