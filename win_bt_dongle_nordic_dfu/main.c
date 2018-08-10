// http://sourceforge.net/projects/libusb-win32
#include <stdio.h>
#include <unistd.h>
#include "bt.h"
#include "bt_usb.h"
#define dump(d, l) do{int i;for(i=0;i<l;i++)printf("%02X ", (unsigned char)d[i]);printf("\n");}while(0)

void recv_cb_server(char *data, int len)
{
	static uint32_t max_size=1<<20, offset, crc;
	if(!bt_cmp(data, CMP_RESET, sizeof(CMP_RESET))){
		hci_send(SET_EVENT_MASK, sizeof(SET_EVENT_MASK));
	}else if(!bt_cmp(data, CMP_SET_EVENT_MASK, sizeof(CMP_SET_EVENT_MASK))){
		hci_send(LE_SET_EVENT_MASK, sizeof(LE_SET_EVENT_MASK));
	}else if(!bt_cmp(data, CMP_LE_SET_EVENT_MASK, sizeof(CMP_LE_SET_EVENT_MASK))){
		hci_send(LE_SET_ADV_PARAM, sizeof(LE_SET_ADV_PARAM));
	}else if(!bt_cmp(data, CMP_LE_SET_ADV_PARAM, sizeof(CMP_LE_SET_ADV_PARAM))){
		hci_send(LE_SET_ADV_DATA, sizeof(LE_SET_ADV_DATA));
	}else if(!bt_cmp(data, CMP_LE_SET_ADV_DATA, sizeof(CMP_LE_SET_ADV_DATA))){
		hci_send(LE_SET_ADV_ENABLE, sizeof(LE_SET_ADV_ENABLE));
	}else if(!bt_cmp(data, CMP_LE_SET_ADV_ENABLE, sizeof(CMP_LE_SET_ADV_ENABLE))){
		puts("Advertise enabled !");
	}else if(!bt_cmp(data, RECV_LE_CONNECTED, sizeof(RECV_LE_CONNECTED))){
		hci_send(SEND_CONN_PARAM_UPDATE_REQ, sizeof(SEND_CONN_PARAM_UPDATE_REQ));
		puts("Device connected !");
	}else if(!bt_cmp(data, RECV_LE_DISCONNECTED, sizeof(RECV_LE_DISCONNECTED))){
		puts("Device disconnected !");
		hci_send(CMD_RESET, sizeof(CMD_RESET));
	}else if(!bt_cmp(data, RECV_IOS_UNKNOWN_L2CAP, sizeof(RECV_IOS_UNKNOWN_L2CAP))){
		hci_send(RSP_IOS_UNKNOWN_L2CAP, sizeof(RSP_IOS_UNKNOWN_L2CAP));
	}else if(!bt_cmp(data, RECV_ATT_EXT_MTU, sizeof(RECV_ATT_EXT_MTU))){
		puts("MTU exchanged !");
		hci_send(RSP_ATT_EXT_MTU, sizeof(RSP_ATT_EXT_MTU));
	}else if(!bt_cmp(data, RECV_GATT_GET_INCLUDE_SERVICE, sizeof(RECV_GATT_GET_INCLUDE_SERVICE))){
		hci_send(RSP_GATT_GET_INCLUDE_SERVICE, sizeof(RSP_GATT_GET_INCLUDE_SERVICE));
	}else if(!bt_cmp(data, RECV_GATT_GET_SERVICE_1, sizeof(RECV_GATT_GET_SERVICE_1))){
		hci_send(RSP_GATT_GET_SERVICE_1, sizeof(RSP_GATT_GET_SERVICE_1));
	}else if(!bt_cmp(data, RECV_GATT_GET_SERVICE_2, sizeof(RECV_GATT_GET_SERVICE_2))){
		hci_send(RSP_GATT_GET_SERVICE_2, sizeof(RSP_GATT_GET_SERVICE_2));
	}else if(!bt_cmp(data, RECV_GATT_GET_SERVICE_3, sizeof(RECV_GATT_GET_SERVICE_3))){
		hci_send(RSP_GATT_GET_SERVICE_3, sizeof(RSP_GATT_GET_SERVICE_3));
	}else if(!bt_cmp(data, RECV_GATT_GET_SERVICE_3_1_IPHONE, sizeof(RECV_GATT_GET_SERVICE_3_1_IPHONE)) ||
			!bt_cmp(data, RECV_GATT_GET_SERVICE_3_1_ANDROID, sizeof(RECV_GATT_GET_SERVICE_3_1_ANDROID))){
		hci_send(RSP_GATT_GET_SERVICE_3_1, sizeof(RSP_GATT_GET_SERVICE_3_1));
	}else if(!bt_cmp(data, RECV_GATT_GET_SERVICE_3_2, sizeof(RECV_GATT_GET_SERVICE_3_2))){
		hci_send(RSP_GATT_GET_SERVICE_3_2, sizeof(RSP_GATT_GET_SERVICE_3_2));
	}else if(!bt_cmp(data, RECV_GATT_GET_SERVICE_4_IPHONE, sizeof(RECV_GATT_GET_SERVICE_4_IPHONE)) ||
			!bt_cmp(data, RECV_GATT_GET_SERVICE_4_ANDROID, sizeof(RECV_GATT_GET_SERVICE_4_ANDROID))){
		hci_send(RSP_GATT_GET_SERVICE_4, sizeof(RSP_GATT_GET_SERVICE_4));
	}else if(!bt_cmp(data, RECV_GATT_GET_SERVICE_5_IPHONE, sizeof(RECV_GATT_GET_SERVICE_5_IPHONE)) ||
			!bt_cmp(data, RECV_GATT_GET_SERVICE_5_ANDROID, sizeof(RECV_GATT_GET_SERVICE_5_ANDROID))){
		hci_send(RSP_GATT_GET_SERVICE_5, sizeof(RSP_GATT_GET_SERVICE_5));
	}else if(!bt_cmp(data, RECV_GATT_READ_CHAR, sizeof(RECV_GATT_READ_CHAR))){
		puts("GATT_READ_CHAR !");
		hci_send(RSP_GATT_READ_CHAR, sizeof(RSP_GATT_READ_CHAR));
	}else if(!bt_cmp(data, RECV_GATT_WRITE_CTRL_POINT, sizeof(RECV_GATT_WRITE_CTRL_POINT))){
		puts("GATT_WRITE CONTROL POINT!");
		hci_send(RSP_GATT_WRITE_CTRL_POINT, sizeof(RSP_GATT_WRITE_CTRL_POINT));
	}else if(!bt_cmp(data, RECV_GATT_WRITE_CHAR, sizeof(RECV_GATT_WRITE_CHAR))){
		puts("GATT_WRITE !");
		hci_send(RSP_GATT_WRITE_CHAR, sizeof(RSP_GATT_WRITE_CHAR));
	}else if(!bt_cmp(data, RECV_GATT_READ, sizeof(RECV_GATT_READ))){
		puts("GATT_READ_DESC !");
		hci_send(RSP_GATT_READ, sizeof(RSP_GATT_READ));
	}
	if(!bt_cmp(data, RECV_GATT_WRITE_CMD_DATA, sizeof(RECV_GATT_WRITE_CMD_DATA))){
		crc = crc32_compute(&data[12], len-12, &crc);
		offset += len-12;
	}
	if(!bt_cmp(data, RECV_GATT_WRITE_CTRL_POINT, sizeof(RECV_GATT_WRITE_CTRL_POINT))){
		if(!memcmp(&data[12], "\x06\x01", 2) || !memcmp(&data[12], "\x06\x02", 2)){ //select
			char tmp[] = {0x02, CONN_HANDLE, 0x00, 0x16, 0x00, 0x12, 0x00, 0x04, 0x00, 0x1b, 0x03, 0x00, 0x60, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
			U32_TO_STREAM(max_size, &tmp[15]);
			U32_TO_STREAM(offset, &tmp[19]);
			U32_TO_STREAM(crc, &tmp[23]);
			hci_send(tmp, sizeof(tmp));
		}else if(!memcmp(&data[12], "\x01\x01", 2) || !memcmp(&data[12], "\x01\x02", 2)){ //create
			char tmp[] = {0x02, CONN_HANDLE, 0x00, 0x0a, 0x00, 0x06, 0x00, 0x04, 0x00, 0x1b, 0x03, 0x00, 0x60, 0x01, 0x01};
			hci_send(tmp, sizeof(tmp));
			offset = 0;
			crc = 0;
		}else if(!memcmp(&data[12], "\x02", 1)){
			char tmp[] = {0x02, CONN_HANDLE, 0x00, 0x0a, 0x00, 0x06, 0x00, 0x04, 0x00, 0x1b, 0x03, 0x00, 0x60, 0x02, 0x01};
			hci_send(tmp, sizeof(tmp));
		}else if(!memcmp(&data[12], "\x03", 1)){
			char tmp[] = {0x02, CONN_HANDLE, 0x00, 0x12, 0x00, 0x0e, 0x00, 0x04, 0x00, 0x1b, 0x03, 0x00, 0x60, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF};
			U32_TO_STREAM(offset, &tmp[15]);
			U32_TO_STREAM(crc, &tmp[19]);
			hci_send(tmp, sizeof(tmp));
		}else if(!memcmp(&data[12], "\x04", 1)){
			char tmp[] = {0x02, CONN_HANDLE, 0x00, 0x0a, 0x00, 0x06, 0x00, 0x04, 0x00, 0x1b, 0x03, 0x00, 0x60, 0x04, 0x01};
			hci_send(tmp, sizeof(tmp));
		}
	}
}
int main(void)
{
	hci_init(USB_LOG_ALL, recv_cb_server);
	hci_send(CMD_RESET, sizeof(CMD_RESET));
	while(1)usleep(50000);
}
