#ifndef __HID_H__
#define __HID_H__
#include "stdbool.h"
#include "bt_encrypt.h"
#include "bt_usb.h"

#define LOCAL_RAND_ADDR 0xd4,0x35,0x1c,0x53,0xfe,0xc0
#define HID_LOCAL_LTK {0x5f}


#define XXXX 0xFE
static uint8_t SEND_GATT_KEY[] = {0x02,XXXX,XXXX,0x0f,0x00,0x0b,0x00,0x04,0x00,0x1B,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};//LC/LS/LA/LG/RC/RS/RA/RG/K1-K6
static uint8_t SEND_GATT_MOUSE_EVT[] = {0x02,XXXX,XXXX,0x0f,0x00,0x0b,0x00,0x04,0x00,0x1B,0x09,0x00,0x00,0x01,0x00,0x01,0x00,0x00};//B1-B8/X/Y/W

#define SEND_KEY_EVT(flag, k) do{if(!HID_ENABLED())break; SET_HANDLE(SEND_GATT_KEY, 1);SEND_GATT_KEY[12]=flag;SEND_GATT_KEY[14]=k;SEND(SEND_GATT_KEY);}while(0)
#define SEND_MOUSE_EVT(button, wheel, x, y) \
	do{ \
		if(!HID_ENABLED())break; \
		SET_HANDLE(SEND_GATT_MOUSE_EVT, 1); \
		int16_t xx = x; int16_t yy = y; \
		if(xx < 0){ \
			*(int16_t*)&SEND_GATT_MOUSE_EVT[13]=((~xx)+1); \
			*(int16_t*)&SEND_GATT_MOUSE_EVT[13] *= -1; \
		}else{ \
			*(int16_t*)&SEND_GATT_MOUSE_EVT[13]=xx; \
		} \
		if(yy < 0){ \
			*(int16_t*)&SEND_GATT_MOUSE_EVT[15]=((~yy)+1); \
			*(int16_t*)&SEND_GATT_MOUSE_EVT[15] *= -1; \
		}else{ \
			*(int16_t*)&SEND_GATT_MOUSE_EVT[15]=yy; \
		} \
		*(uint8_t*)&SEND_GATT_MOUSE_EVT[12]=(button); \
		*(uint8_t*)&SEND_GATT_MOUSE_EVT[17]=(wheel); \
		SEND(SEND_GATT_MOUSE_EVT); \
	}while(0)

void send_reset(void);
bool HID_ENABLED(void);
void SET_HANDLE(uint8_t *data, int offset);

void send_hid(uint8_t *buf, uint8_t len);
void bt_recv_cb(uint8_t *d, int len);

#define SEND(data) send_hid(data, sizeof(data))

#endif /* __HID_H__ */
