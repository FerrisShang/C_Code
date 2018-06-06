#include <lusb0_usb.h> // ref: http://sourceforge.net/projects/libusb-win32
#include <stdio.h>

// Device vendor and product id.
#define MY_VID 0x0a5c
#define MY_PID 0x21ec
// Device configuration and interface id.
#define MY_CONFIG 1
#define MY_INTF   0
// Device endpoint(s)
#define EP_IN  (USB_ENDPOINT_IN | USB_RECIP_INTERFACE)
#define EP_OUT (USB_TYPE_CLASS | USB_ENDPOINT_OUT)
// Device of bytes to transfer.
#define BUF_SIZE 64

usb_dev_handle *open_dev(void);
static int transfer_bulk_async(usb_dev_handle *dev, int ep, char *bytes, int size, int timeout);

int main(void)
{
    usb_dev_handle *dev = NULL; /* the device handle */
    char tmp[BUF_SIZE] = {0x03, 0x0c, 0x00};
    int ret;
    void* async_read_context = NULL;
    void* async_write_context = NULL;

    usb_init(); /* initialize the library */
    usb_find_busses(); /* find all busses */
    usb_find_devices(); /* find all connected devices */
    if (!(dev = open_dev())){
        printf("error opening device: \n%s\n", usb_strerror());
        return 0;
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
    ret = usb_control_msg(dev, EP_OUT, 0, 0, 0, tmp, 3, 1000);
    printf("-- %d -- : ret=%d\n", __LINE__, ret);
    ret = transfer_bulk_async(dev, EP_IN, tmp, sizeof(tmp), 5000);
    printf("-- %d -- : ret=%d\n", __LINE__, ret);
    int i; for(i=0;i<6;i++)printf("%02x ", tmp[i]);printf("\n");
    usb_release_interface(dev, 0);
    if (dev){
        usb_close(dev);
    }
    return 0;
}

usb_dev_handle *open_dev(void)
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

static int transfer_bulk_async(usb_dev_handle *dev,
                               int ep,
                               char *bytes,
                               int size,
                               int timeout)
{
    // Each async transfer requires it's own context. A transfer
    // context can be re-used.  When no longer needed they must be
    // freed with usb_free_async().
    //
    void* async_context = NULL;
    int ret;
    // Setup the async transfer.  This only needs to be done once
    // for multiple submit/reaps. (more below)
    //
    ret = usb_bulk_setup_async(dev, &async_context, ep);
    if (ret < 0){
        printf("error usb_bulk_setup_async:\n%s\n", usb_strerror());
        goto Done;
    }
    // Submit this transfer.  This function returns immediately and the
    // transfer is on it's way to the device.
    ret = usb_submit_async(async_context, bytes, size);
    if (ret < 0){
        printf("error usb_submit_async:\n%s\n", usb_strerror());
        usb_free_async(&async_context);
        goto Done;
    }
    // Wait for the transfer to complete.  If it doesn't complete in the
    // specified time it is cancelled.  see also usb_reap_async_nocancel().
    //
    ret = usb_reap_async(async_context, timeout);
    // Free the context.
    usb_free_async(&async_context);
Done:
    return ret;
}
