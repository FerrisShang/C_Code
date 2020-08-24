#ifndef __EASYBLE_H__
#define __EASYBLE_H__

#include "eb_config.h"
#include "eb_hci.h"
#include "eb_l2cap.h"
#include "eb_gap.h"
#include "eb_att.h"
#include "eb_gattc.h"
#include "eb_gatts.h"
#include "eb_smp.h"

#define EB_TIMER_NUM 4

typedef struct {
    uint16_t evt_id;
    union{
        eb_gap_event_t gap;
        eb_gatts_event_t gatts;
        eb_gattc_event_t gattc;
    };
}eb_event_t;

/************ Porting interface ************/
void eb_init(void(*ble_event_cb)(eb_event_t *));
void eb_schedule(void);
void eb_h4_recv(uint8_t *data, int len);
void eb_h4_send(uint8_t *data, int len);
void eb_event(eb_event_t *param);
void eb_set_timer(uint8_t id, uint32_t tout_ms, bool(*cb)(uint8_t id, void*p), void*param);
void eb_del_timer(uint8_t id);

#endif /* __EASYBLE_H__ */
