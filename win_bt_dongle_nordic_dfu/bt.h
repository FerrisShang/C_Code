#ifndef __BT_H__
#define __BT_H__

#define IGBT         0x5f
#define CONN_HANDLE  0x40

#include "stdio.h"
#include "stdint.h"
#define U32_TO_STREAM(u32, p) do{((char*)(p))[0]=(u32>>0)&0xFF;((char*)(p))[1]=(u32>>8)&0xFF;((char*)(p))[2]=(u32>>16)&0xFF;((char*)(p))[3]=(u32>>24)&0xFF;}while(0)

static int bt_cmp(char *data, char *tmpl, int len){
	while(len--){ if(*tmpl != IGBT && *data != *tmpl) return -1; else {data++;tmpl++;}} return 0;
}

static  uint32_t crc32_compute(uint8_t const * p_data, uint32_t size, uint32_t const * p_crc)
{
    uint32_t crc;
    crc = (p_crc == NULL) ? 0xFFFFFFFF : ~(*p_crc);
    for (uint32_t i = 0; i < size; i++) {
        crc = crc ^ p_data[i];
        for (uint32_t j = 8; j > 0; j--) crc = (crc >> 1) ^ (0xEDB88320U & ((crc & 1) ? 0xFFFFFFFF : 0));
    }
    return ~crc;
}


char RECV_LE_CONNECTED[] = {0x04, 0x3e, 0x13, 0x01, 0x00};
char RECV_LE_DISCONNECTED[] = {0x04, 0x05, 0x04, 0x00, 0x40};

char CMD_RESET[] = {0x01, 0x03, 0x0c, 0x00};
char CMP_RESET[] = {0x04, 0x0e, 0x04, 0x01, 0x03, 0x0c, 0x00};

char SET_EVENT_MASK[] = {0x01, 0x01, 0x0c, 0x08, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0x3d};
char CMP_SET_EVENT_MASK[] = {0x04, 0x0e, 0x04, 0x01, 0x01, 0x0c, 0x00};

char LE_SET_EVENT_MASK[] = {0x01, 0x01, 0x20, 0x08, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00};
char CMP_LE_SET_EVENT_MASK[] = {0x04, 0x0e, 0x04, 0x01, 0x01, 0x20, 0x00};

char LE_SET_ADV_PARAM[] = {0x01, 0x06, 0x20, 0x0f, 0x40, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00};
char CMP_LE_SET_ADV_PARAM[] = {0x04, 0x0e, 0x04, 0x01, 0x06, 0x20, 0x00};

char LE_SET_ADV_DATA[] = {0x01, 0x08, 0x20, 0x20, 0x19, 0x0b, 0x09, 0x55, 0x53, 0x42, 0x20, 0x44, 0x6f, 0x6e, 0x67, 0x6c, 0x65, 0x03, 0x19, 0xc2, 0x03, 0x02, 0x01, 0x06, 0x03, 0x03, 0x12, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
char CMP_LE_SET_ADV_DATA[] = {0x04, 0x0e, 0x04, 0x01, 0x08, 0x20, 0x00};

char LE_SET_ADV_ENABLE[] = {0x01, 0x0a, 0x20, 0x01, 0x01};
char CMP_LE_SET_ADV_ENABLE[] = {0x04, 0x0e, 0x04, 0x01, 0x0a, 0x20, 0x00};

char LE_SET_SCAN_ENABLE[] = {0x01, 0x0c, 0x20, 0x02, 0x01, 0x00};
char CMP_LE_SET_SCAN_ENABLE[] = {0x04, 0x0e, 0x04, 0x01, 0x0c, 0x20, 0x00};

char LE_SET_SCAN_DISABLE[] = {0x01, 0x0c, 0x20, 0x02, 0x00, 0x00};
char CMP_LE_SET_SCAN_DISABLE[] = {0x04, 0x0e, 0x04, 0x01, 0x0c, 0x20, 0x00};

char RECV_IOS_UNKNOWN_L2CAP[] = {0x02, IGBT, IGBT, 0x0b, 0x00, 0x07, 0x00, 0x3a, 0x00};
char RSP_IOS_UNKNOWN_L2CAP[] = {0x02, CONN_HANDLE, 0x00, 0x0e, 0x00, 0x0a, 0x00, 0x05, 0x00, 0x01, 0x00, 0x06, 0x00, 0x02, 0x00, 0x3a, 0x00, 0x00, 0x00};

char RECV_ATT_EXT_MTU[] = {0x02, IGBT, IGBT, 0x07, 0x00, 0x03, 0x00, 0x04, 0x00, 0x02, IGBT, IGBT};
char RSP_ATT_EXT_MTU[] = {0x02, CONN_HANDLE, 0x00, 0x07, 0x00, 0x03, 0x00, 0x04, 0x00, 0x03, 0x17, 0x00};

char RECV_GATT_GET_SERVICE_1[] = {0x02, IGBT, IGBT, 0x0b, 0x00, 0x07, 0x00, 0x04, 0x00, 0x10, 0x01, 0x00, 0xff, 0xff, 0x00, 0x28};
char RSP_GATT_GET_SERVICE_1[] = {0x02, CONN_HANDLE, 0x00, 0x0C, 0x00, 0x08, 0x00, 0x04, 0x00, 0x11, 0x06, 0x01, 0x00, 0x07, 0x00, 0x59, 0xFE};

char RECV_GATT_GET_SERVICE_2[] = {0x02, IGBT, IGBT, 0x0b, 0x00, 0x07, 0x00, 0x04, 0x00, 0x10, 0x08, 0x00, 0xff, 0xff, 0x00, 0x28};
char RSP_GATT_GET_SERVICE_2[] = {0x02, CONN_HANDLE, 0x00, 0x09, 0x00, 0x05, 0x00, 0x04, 0x00, 0x01, 0x10, 0x08, 0x00, 0x0a};

char RECV_GATT_GET_INCLUDE_SERVICE[] = {0x02, IGBT, IGBT, 0x0b, 0x00, 0x07, 0x00, 0x04, 0x00, 0x08, IGBT, IGBT, IGBT, IGBT, 0x02, 0x28};
char RSP_GATT_GET_INCLUDE_SERVICE[] = {0x02, CONN_HANDLE, 0x00, 0x09, 0x00, 0x05, 0x00, 0x04, 0x00, 0x01, 0x08, 0x06, 0x00, 0x0a};
//Find characteristic 
char RECV_GATT_GET_SERVICE_3[] = {0x02, IGBT, IGBT, 0x0b, 0x00, 0x07, 0x00, 0x04, 0x00, 0x08, 0x01, 0x00, 0x07, 0x00, 0x03, 0x28};
char RSP_GATT_GET_SERVICE_3[] = {0x02, CONN_HANDLE, 0x00, 0x1b, 0x00, 0x17, 0x00, 0x04, 0x00, 0x09, 0x15, 0x02, 0x00, 0x18, 0x03, 0x00, 0x50, 0xEA, 0xDA, 0x30, 0x88, 0x83, 0xB8, 0x9F, 0x60, 0x4F, 0x15, 0xF3, 0x01, 0x00, 0xC9, 0x8E};
//Find characteristic 2
char RECV_GATT_GET_SERVICE_3_1_IPHONE[] = {0x02, IGBT, IGBT, 0x0b, 0x00, 0x07, 0x00, 0x04, 0x00, 0x08, 0x04, 0x00, 0x07, 0x00, 0x03, 0x28};
char RECV_GATT_GET_SERVICE_3_1_ANDROID[] = {0x02, IGBT, IGBT, 0x0b, 0x00, 0x07, 0x00, 0x04, 0x00, 0x08, 0x03, 0x00, 0x07, 0x00, 0x03, 0x28};
char RSP_GATT_GET_SERVICE_3_1[] = {0x02, CONN_HANDLE, 0x00, 0x1b, 0x00, 0x17, 0x00, 0x04, 0x00, 0x09, 0x15, 0x05, 0x00, 0x14, 0x06, 0x00, 0x50, 0xEA, 0xDA, 0x30, 0x88, 0x83, 0xB8, 0x9F, 0x60, 0x4F, 0x15, 0xF3, 0x02, 0x00, 0xC9, 0x8E};
//Find characteristic 0x05~0x05, response error: not found
char RECV_GATT_GET_SERVICE_3_2[] = {0x02, IGBT, IGBT, 0x0b, 0x00, 0x07, 0x00, 0x04, 0x00, 0x08, IGBT, 0x00, IGBT, 0x00, 0x03, 0x28};
char RSP_GATT_GET_SERVICE_3_2[] = {0x02, CONN_HANDLE, 0x00, 0x09, 0x00, 0x05, 0x00, 0x04, 0x00, 0x01, 0x08, 0x06, 0x00, 0x0a};
//Find descriptor
char RECV_GATT_GET_SERVICE_4_IPHONE[] = {0x02, IGBT, IGBT, 0x09, 0x00, 0x05, 0x00, 0x04, 0x00, 0x04, 0x04, 0x00, 0x04, 0x00};
char RECV_GATT_GET_SERVICE_4_ANDROID[] = {0x02, IGBT, IGBT, 0x09, 0x00, 0x05, 0x00, 0x04, 0x00, 0x01, 0x07, 0x00, 0x04, 0x00};
char RSP_GATT_GET_SERVICE_4[] = {0x02, CONN_HANDLE, 0x00, 0x0a, 0x00, 0x06, 0x00, 0x04, 0x00, 0x05, 0x01, 0x04, 0x00, 0x02, 0x29};
//Find descriptor
char RECV_GATT_GET_SERVICE_5_IPHONE[] = {0x02, IGBT, IGBT, 0x09, 0x00, 0x05, 0x00, 0x04, 0x00, 0x04, 0x07, 0x00, 0x07, 0x00};
char RECV_GATT_GET_SERVICE_5_ANDROID[] = {0x02, IGBT, IGBT, 0x09, 0x00, 0x05, 0x00, 0x04, 0x00, 0x04, 0x06, 0x00, 0x07, 0x00};
char RSP_GATT_GET_SERVICE_5[] = {0x02, CONN_HANDLE, 0x00, 0x0a, 0x00, 0x06, 0x00, 0x04, 0x00, 0x05, 0x01, 0x07, 0x00, 0x02, 0x29};

char RECV_GATT_READ_CHAR[] = {0x02, IGBT, IGBT, 0x07, 0x00, 0x03, 0x00, 0x04, 0x00, 0x0a, 0x03, 0x00};
char RSP_GATT_READ_CHAR[] = {0x02, CONN_HANDLE, 0x00, 0x0f, 0x00, 0x0b, 0x00, 0x04, 0x00, 0x0b, 0x4d, 0x79, 0x54, 0x65, 0x73, 0x74, 0x44, 0x61, 0x74, 0x61};
// DFU Select
char RECV_GATT_WRITE_CTRL_POINT[] = {0x02, IGBT, IGBT, IGBT, 0x00, IGBT, 0x00, 0x04, 0x00, 0x12, 0x03, 0x00};
char RSP_GATT_WRITE_CTRL_POINT[] = {0x02, CONN_HANDLE, 0x00, 0x05, 0x00, 0x01, 0x00, 0x04, 0x00, 0x13};

char RECV_GATT_WRITE_CMD_DATA[] = {0x02, IGBT, IGBT, IGBT, 0x00, IGBT, 0x00, 0x04, 0x00, 0x52, 0x06, 0x00};

char RECV_GATT_WRITE_CHAR[] = {0x02, IGBT, IGBT, IGBT, IGBT, IGBT, IGBT, 0x04, 0x00, 0x12}; // Ignore write handle & data (last 3 bytes)
char RSP_GATT_WRITE_CHAR[] = {0x02, CONN_HANDLE, 0x00, 0x05, 0x00, 0x01, 0x00, 0x04, 0x00, 0x13};

char RECV_GATT_READ[] = {0x02, IGBT, IGBT, 0x07, 0x00, 0x03, 0x00, 0x04, 0x00, 0x0a, IGBT, 0x00};
char RSP_GATT_READ[] = {0x02, CONN_HANDLE, 0x00, 0x07, 0x00, 0x03, 0x00, 0x04, 0x00, 0x0b, 0x00, 0x00};

char SEND_GATT_READ_REQ[] = {0x02, CONN_HANDLE, 0x00, 0x07, 0x00, 0x03, 0x00, 0x04, 0x00, 0x0a, 0x02, 0x00};

char SEND_SMP_PAIRING_REQ[] = {0x02, CONN_HANDLE, 0x00, 0x0B, 0x00, 0x07, 0x00, 0x06, 0x00, 0x01, 0x03, 0x00, 0x01, 0x10, 0x03, 0x03};

char SEND_CONN_PARAM_UPDATE_REQ[] = {0x02, CONN_HANDLE, 0x00, 0x10, 0x00, 0x0C, 0x00, 0x05, 0x00, 0x12, 0x00, 0x08, 0x00, 0x0C, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x01};

char RECV_ADV_REPORT[] = {0x04, 0x3e, IGBT, 0x02, 0x01, 0x00};

char RECV_L2CAP_PARAM_UPDATE_REQ[] = {0x02, IGBT, IGBT, 0x10, 0x00, 0x0c, 0x00, 0x05, 0x00, 0x12, IGBT, 0x08, 0x00};
char RSP_L2CAP_PARAM_UPDATE[] = {0x02, CONN_HANDLE, 0x00, 0x0a, 0x00, 0x06, 0x00, 0x05, 0x00, 0x13, 0x02, 0x02, 0x00, 0x00, 0x00};


#endif /* __BT_H__ */

