#ifndef __EB_SMP_H__
#define __EB_SMP_H__

#include "eb_config.h"

void eb_smp_init(void);
void eb_smp_handler(uint8_t *data, uint16_t len);
void eb_smp_auth(uint16_t conn_hd);


#endif /* __EB_SMP_H__ */
