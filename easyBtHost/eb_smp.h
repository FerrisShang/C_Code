#ifndef __EB_SMP_H__
#define __EB_SMP_H__

#include "eb_config.h"

void eb_smp_init(void);
void eb_smp_handler(uint8_t *data, uint16_t len);


#endif /* __EB_SMP_H__ */
