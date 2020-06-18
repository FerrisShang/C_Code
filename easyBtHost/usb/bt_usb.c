// sourceforge.net/projects/libusb-win32
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include "bt_usb.h"
#include "btsnoop_rec.h"

#define dump(d, l) do{int i;for(i=0;i<l;i++)printf("%02X ", (unsigned char)d[i]);printf("\n");fflush(stdout);}while(0)
#define TRAN_TOUT (2000)
#define LOG_IN    (1)
#define LOG_OUT   (0)

static int _log_flag;
static FILE *bt_snoop;
static void (*_recv_cb)(uint8_t *data, int len);
static pthread_mutex_t log_lock, callback_lock;
static usb_dev_handle *usb_dev=NULL;

static void log_data(uint8_t *data, int len, int dir)
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
	uint8_t buf[1024];
	while(1){
		int res = usb_hci_recv(buf, 1024, ep);
		if(res > 0){
	        pthread_mutex_lock(&callback_lock);
			_recv_cb(buf, res);
	        pthread_mutex_unlock(&callback_lock);
		}else{
			usleep(1000);
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
				usb_dev_handle *usb_dev = usb_open(dev);
				if (usb_set_configuration(usb_dev, MY_CONFIG) < 0){
					//printf("error setting config #%d: %s\n", MY_CONFIG, usb_strerror());
					usb_close(usb_dev);
					continue;
				}
				if (usb_claim_interface(usb_dev, 0) < 0){
					//printf("error claiming interface #%d:\n%s\n", MY_INTF, usb_strerror());
					usb_close(usb_dev);
					continue;
				}
				return usb_dev;
			}
		}
	}
	return NULL;
}

static usb_dev_handle *get_usb_dev(void)
{
	if(usb_dev) return usb_dev;
	usb_init(); /* initialize the library */
	usb_find_busses(); /* find all busses */
	usb_find_devices(); /* find all connected devices */
	if (!(usb_dev = open_dev())){
		printf("error opening device: \n%s\n", usb_strerror());
		return NULL;
	}
	return usb_dev;
}

usb_dev_handle *usb_hci_init(int log_flag, void (*recv_cb)(uint8_t *data, int len))
{
	usb_dev_handle *ret = get_usb_dev();
	_log_flag = log_flag;
	_recv_cb = recv_cb;
	pthread_mutex_init(&log_lock, NULL);
	pthread_mutex_init(&callback_lock, NULL);
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

static bool get_usb_dev_flag;
void *get_usb_dev_th(void *p)
{
	while(!usb_dev){
		get_usb_dev();
		usleep(1000000);
	}
	get_usb_dev_flag = 0;
	((void(*)(void))p)();
}

void usb_hci_reinit(void (*cb)(void))
{
	if(get_usb_dev_flag) return;
	get_usb_dev_flag = 1;
	pthread_t th;
	if(usb_dev){ usb_close(usb_dev); }
	usb_dev = NULL;
	pthread_create(&th, 0, get_usb_dev_th, cb);
}

int usb_hci_send(uint8_t *data, int len)
{
	int ret;
	assert(len >= 1 && (data[0] == 0x01 || data[0] == 0x02));
	if(!usb_dev) return -1;
	if(data[0] == 0x01){ //cmd
		ret = usb_control_msg(usb_dev, USB_EP_CMD_OUT, 0, 0, 0, (char*)(data+1), len-1, TRAN_TOUT);
	}else if(data[0] == 0x02){ //acl
		ret = usb_bulk_write(usb_dev, USB_EP_ACL_OUT, (char*)(data+1), len-1, TRAN_TOUT);
	}
	if(ret > 0) log_data(data, len, LOG_OUT);
	return ret;
}

int usb_hci_recv(uint8_t *data, int len, int endpoint)
{
	assert(len > 0 && (endpoint == USB_EP_EVT_IN || endpoint == USB_EP_ACL_IN));
	if(!usb_dev) return -1;
	int ret = usb_bulk_read(get_usb_dev(), endpoint, (char*)(data+1), len-1, TRAN_TOUT);
	if(ret > 0){
		data[0] = endpoint == USB_EP_EVT_IN ? 0x04 : 0x02;
		log_data(data, ret + 1, LOG_IN);
		return ret + 1;
	}else{
		return ret;
	}
}
