#ifndef __EB_CONFIG_H__
#define __EB_CONFIG_H__

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>


#define HANDLE_INVALID 0xFFFF
#define EB_ATT_MTU_DEFAULT 23

typedef uint8_t bdaddr_t[6];
enum{
    EB_HCI_ID,
    EB_L2CAP_ID,
    EB_GAP_ID,
    EB_ATT_ID,
    EB_GATTC_ID,
    EB_GATTS_ID,
    EB_SMP_ID,
};


#endif /* __EB_CONFIG_H__ */


