#ifndef __EB_L2CAP_H__
#define __EB_L2CAP_H__

#include "eb_config.h"

#define L2CAP_PKG_MAX_NUM 10

void eb_l2cap_init(void);
void eb_l2cap_handler(uint8_t *data, uint16_t len);
void eb_l2cap_update_conn_param(uint16_t handle, uint16_t min_intv, uint16_t max_intv, uint16_t latency, uint16_t timeout);
void eb_l2cap_send(uint8_t *data, uint32_t len);
void l2cap_packet_inc(void);
void l2cap_packet_dec(int num);
int l2cap_packet_num(void);

#endif /* __EB_L2CAP_H__ */
