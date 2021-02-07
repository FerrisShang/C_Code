#ifndef __ONMICRO_DFU_CLIENT_H__
#define __ONMICRO_DFU_CLIENT_H__
#include <stdint.h>

#define DFU_INVALID_CODE             0x00
#define DFU_SUCCESS                  0x01
#define DFU_OPCODE_NOT_SUPPORT       0x02
#define DFU_INVALID_PARAMETER        0x03
#define DFU_INSUFFICIENT_RESOURCES   0x04
#define DFU_INVALID_OBJECT           0x05
#define DFU_UNSUPPORTED_TYPE         0x07
#define DFU_OPERATION_NOT_PERMITTED  0x08
#define DFU_OPERATION_FAILED         0x0A
#define DFU_UPDATE_ABORT             0x80
#define DFU_CRC_NOT_MATCH            0x81

enum dfu_pkg_type {
    DFU_PKG_TYPE_CMD  = 1, // ext: .dat
    DFU_PKG_TYPE_DATA = 2, // ext: .bin
};

enum dfu_gatt_type {
    DFU_GATT_TYPE_CTRL  = 1,
    DFU_GATT_TYPE_DATA = 2,
};

enum dfu_evt {
    DFU_EVT_PROG,
    DFU_EVT_END,
};

// API
#include "stdio.h"
#define dfu_log(...) printf(__VA_ARGS__)
#define dfu_assert(...)

void dfu_client_start(uint16_t mtu, uint8_t pkg_max_num, uint8_t prn);
void dfu_client_abort(void);
void dfu_client_gatt_recv(uint8_t *data, uint32_t length);
void dfu_client_can_send(uint8_t pkg_num);

// PORT
void dfu_client_evt_cb(uint8_t evt, void* param);
void dfu_client_get_info_cb(uint8_t pkg_type, uint32_t *length);
void dfu_client_get_data_cb(uint8_t pkg_type, uint32_t offset, uint32_t *max_len, uint8_t *data);
void dfu_client_gatt_send_cb(uint8_t gatt_type, uint8_t *data, uint32_t length);

#endif /* __ONMICRO_DFU_CLIENT_H__ */
