// http://sourceforge.net/projects/libusb-win32
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include "bt_usb.h"
#include "btsnoop_rec.h"

#define dump(d, l) do{int i;for(i=0;i<l;i++)printf("%02X ", (unsigned char)d[i]);printf("\n");}while(0)
#define TRAN_TOUT (2000)
#define LOG_IN    (1)
#define LOG_OUT   (0)

static int _log_flag;
static FILE *bt_snoop;
static void (*_recv_cb)(char *data, int len);
static pthread_mutex_t log_lock;

static void log_data(char *data, int len, int dir)
{
	pthread_mutex_lock(&log_lock);
	if((_log_flag & USB_LOG_OUTPUT) && dir == LOG_IN) { printf(">>> "); dump(data, len); }
	else if((_log_flag & USB_LOG_OUTPUT) && dir == LOG_OUT){ printf("<<< "); dump(data, len); }
	if(_log_flag & USB_LOG_BTSNOOP) { record_btsnoop(bt_snoop, data, len, dir); }
	pthread_mutex_unlock(&log_lock);
}

static void* hci_read_th(void *p)
{
	int ep = *(int*)p;
	char buf[1024];
	while(1){
		int res = hci_recv(buf, 1024, ep);
		if(res > 0){
			_recv_cb(buf, res);
		}else{
			usleep(10000);
		}
	}
}
static usb_dev_handle *open_dev(void)
{
    struct usb_bus *bus;
    struct usb_device *dev;
    for (bus = usb_get_busses(); bus; bus = bus->next){
        for (dev = bus->devices; dev; dev = dev->next){
            if (dev->descriptor.idVendor == MY_VID && dev->descriptor.idProduct == MY_PID){
                return usb_open(dev);
            }
        }
    }
    return NULL;
}

static usb_dev_handle *get_usb_dev(void)
{
	static usb_dev_handle *dev=NULL;
	if(dev) return dev;
    usb_init(); /* initialize the library */
    usb_find_busses(); /* find all busses */
    usb_find_devices(); /* find all connected devices */
    if (!(dev = open_dev())){
        printf("error opening device: \n%s\n", usb_strerror());
        return NULL;
    }
    if (usb_set_configuration(dev, MY_CONFIG) < 0){
        printf("error setting config #%d: %s\n", MY_CONFIG, usb_strerror());
        usb_close(dev);
        return 0;
    }
    if (usb_claim_interface(dev, 0) < 0){
        printf("error claiming interface #%d:\n%s\n", MY_INTF, usb_strerror());
        usb_close(dev);
        return 0;
    }
	return dev;
}

usb_dev_handle *hci_init(int log_flag, void (*recv_cb)(char *data, int len))
{
	usb_dev_handle *ret = get_usb_dev();
	_log_flag = log_flag;
	_recv_cb = recv_cb;
	pthread_mutex_init(&log_lock, NULL);
	if(_log_flag & USB_LOG_BTSNOOP){
		bt_snoop = create_btsnoop_rec(BT_SNOOP_PATH);
	}
	if(_recv_cb){
		pthread_t th;
		static int th1_ep = (USB_EP_EVT_IN), th2_ep = (USB_EP_ACL_IN);
		pthread_create(&th, 0, hci_read_th, &th1_ep);
		pthread_create(&th, 0, hci_read_th, &th2_ep);
	}
	return ret;
}
int hci_send(char *data, int len)
{
	int ret;
	assert(len >= 1 && (data[0] == 0x01 || data[0] == 0x02));
	if(data[0] == 0x01){ //cmd
    	ret = usb_control_msg(get_usb_dev(), USB_EP_CMD_OUT, 0, 0, 0, data+1, len-1, TRAN_TOUT);
	}else if(data[0] == 0x02){ //acl
    	ret = usb_bulk_write(get_usb_dev(), USB_EP_ACL_OUT, data+1, len-1, TRAN_TOUT);
	}
	if(ret > 0) log_data(data, len, LOG_OUT);
	return ret;
}

int hci_recv(char *data, int len, int endpoint)
{
	assert(len > 0 && (endpoint == USB_EP_EVT_IN || endpoint == USB_EP_ACL_IN));
	int ret = usb_bulk_read(get_usb_dev(), endpoint, data+1, len-1, TRAN_TOUT);
	if(ret > 0){
		data[0] = endpoint == USB_EP_EVT_IN ? 0x04 : 0x02;
		log_data(data, ret + 1, LOG_IN);
		return ret + 1;
	}else{
		return ret;
	}
}
