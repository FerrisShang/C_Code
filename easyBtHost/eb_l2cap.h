#ifndef __EB_L2CAP_H__
#define __EB_L2CAP_H__

#include "eb_config.h"


void eb_l2cap_init(void);
void eb_l2cap_handler(uint8_t *data, uint16_t len);
void eb_l2cap_update_conn_param(uint16_t handle, uint16_t max_intv, uint16_t min_intv, uint16_t latency, uint16_t timeout);

#endif /* __EB_L2CAP_H__ */
