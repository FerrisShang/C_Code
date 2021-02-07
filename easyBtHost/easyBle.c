#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "easyBle.h"
#include "bt_usb.h"
#include "hci_cache.h"

static void(*ble_event_callback)(eb_event_t *param);

typedef struct {
    bool enabled;
    uint32_t tout_ms;
    uint32_t tout_time;
    void *param;
    bool(*cb)(uint8_t id, void*p);
}eb_timer_t;
static eb_timer_t timer_rec[EB_TIMER_NUM];
extern void eb_check_timer(void);

void eb_init(void(*ble_event_cb)(eb_event_t *))
{
    assert(ble_event_cb);
    ble_event_callback = ble_event_cb;
    eb_l2cap_init();
    eb_gap_init();
    eb_att_init();
    eb_gattc_init();
    eb_gatts_init();
    eb_smp_init();
    eb_hci_init();
    // Init hci interface
    hci_buf_clear();
    usb_hci_deinit();
    usb_hci_init(USB_LOG_BTSNOOP, eb_h4_recv);
    eb_h4_send((uint8_t*)"\x01\x03\x0C\x00", 4);
    usleep(1000);
}

void eb_schedule(void)
{
    if(hci_buf_cnt()){
        uint32_t len;
        uint8_t *data = hci_buf_get(&len);
        eb_hci_handler(data, len);
    }else{
        eb_check_timer();
        usleep(1000);
    }
}

void eb_h4_recv(uint8_t *data, int len)
{
    hci_buf_in(data, len);
}

void eb_h4_send(uint8_t *data, int len)
{
    if(data[0] == 2){
        eb_l2cap_send(data, len);
    }else{
        eb_hci_send(data, len);
    }
}
void eb_event(eb_event_t *param)
{
    ble_event_callback(param);
}
void eb_set_timer(uint8_t id, uint32_t tout_ms, bool(*cb)(uint8_t id, void*p), void*param)
{
    assert(id < EB_TIMER_NUM);
    assert(cb);
    assert(tout_ms < 0x7FFFFFFF);
    timer_rec[id].enabled = true;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    timer_rec[id].tout_ms = tout_ms;
    timer_rec[id].tout_time = tv.tv_sec*1000+tv.tv_usec/1000+tout_ms;
    timer_rec[id].cb = cb;
    timer_rec[id].param = param;
}
void eb_del_timer(uint8_t id)
{
    timer_rec[id].enabled = false;
}
void eb_check_timer(void)
{
    int i;
    for(i=0;i<EB_TIMER_NUM;i++){
        if(timer_rec[i].enabled){
            struct timeval tv;
            gettimeofday(&tv, NULL);
            uint32_t now = tv.tv_sec*1000+tv.tv_usec/1000;
            if(now - timer_rec[i].tout_time < 0x7FFFFFFF){
                if(!timer_rec[i].cb(i, timer_rec[i].param)){
                    timer_rec[i].enabled = false;
                }else{
                    timer_rec[i].tout_time += timer_rec[i].tout_ms;
                }
            }
        }
    }
}



