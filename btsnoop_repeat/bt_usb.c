#include <stdio.h>
#include <stdlib.h>
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

#ifndef __LINUX__

static USB_DEV_T *open_dev(void)
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

static USB_DEV_T *get_usb_dev(void)
{
	static USB_DEV_T *dev=NULL;
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

#else

USB_DEV_T *get_usb_dev(void)
{
	static libusb_device_handle* dev = NULL;
	if(dev) return dev;
	int r = libusb_init(NULL);
	if (r < 0) { fprintf(stderr, "failed to initialise libusb\n"); exit(1); }
	dev = libusb_open_device_with_vid_pid(NULL, MY_VID, MY_PID);
	if (!dev){ fprintf(stderr, "Could not find/open device\n"); goto out; }
	libusb_detach_kernel_driver(dev, 0);
	r = libusb_set_configuration(dev, MY_CONFIG);
	if (r < 0){ printf("error setting config #%d: %s\n", MY_CONFIG, libusb_strerror(r)); goto out; }
	r = libusb_claim_interface(dev, 0);
	if (r < 0) { fprintf(stderr, "usb_claim_interface error %d\n", r); goto out; }
	printf("claimed interface\n");
	return dev;
out:
	libusb_close(dev);
	libusb_exit(NULL);
	exit(1);
}

int hci_send(char *data, int len)
{
	int ret;
	USB_DEV_T *dev = get_usb_dev();
	assert(len >= 1 && (data[0] == 0x01 || data[0] == 0x02));
	if(data[0] == 0x01){ //cmd
		ret = libusb_control_transfer(dev, USB_EP_CMD_OUT, 0, 0, 0, data+1, len-1, TRAN_TOUT);
		if(ret > 0) log_data(data, len, LOG_OUT);
	}else if(data[0] == 0x02){ //acl
		ret = libusb_bulk_transfer(dev, USB_EP_ACL_OUT, data+1, len-1, &len, TRAN_TOUT);
		if(!ret) log_data(data, len+1, LOG_OUT);
	}
	return ret;
}

int hci_recv(char *data, int len, int endpoint)
{
	assert(len > 0 && (endpoint == USB_EP_EVT_IN || endpoint == USB_EP_ACL_IN));
	int recv_len = -1;
	USB_DEV_T *dev = get_usb_dev();
	if(!dev){puts("Error! usb_recv");while(1);}
	libusb_bulk_transfer(dev, endpoint, data+1, len-1, &recv_len, TRAN_TOUT);
	if(recv_len > 0){
		data[0] = endpoint == USB_EP_EVT_IN ? 0x04 : 0x02;
		log_data(data, recv_len + 1, LOG_IN);
		return recv_len + 1;
	}
	return recv_len;
}
#endif

USB_DEV_T *hci_init(int log_flag, void (*recv_cb)(char *data, int len))
{
	USB_DEV_T *ret = get_usb_dev();
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
