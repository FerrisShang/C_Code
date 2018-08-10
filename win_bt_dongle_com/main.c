// http://sourceforge.net/projects/libusb-win32
#include <stdio.h>
#include <unistd.h>
#include "bt.h"
#include "bt_usb.h"
#define dump(d, l) do{int i;for(i=0;i<l;i++)printf("%02X ", (unsigned char)d[i]);printf("\n");}while(0)

void recv_cb_server(char *data, int len)
{
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
		puts("Device connected !");
	}else if(!bt_cmp(data, RECV_LE_DISCONNECTED, sizeof(RECV_LE_DISCONNECTED))){
		puts("Device disconnected !");
		hci_send(CMD_RESET, sizeof(CMD_RESET));
	}else if(!bt_cmp(data, RECV_IOS_UNKNOWN_L2CAP, sizeof(RECV_IOS_UNKNOWN_L2CAP))){
		hci_send(RSP_IOS_UNKNOWN_L2CAP, sizeof(RSP_IOS_UNKNOWN_L2CAP));
	}else if(!bt_cmp(data, RECV_ATT_EXT_MTU, sizeof(RECV_ATT_EXT_MTU))){
		puts("MTU exchanged !");
		hci_send(RSP_ATT_EXT_MTU, sizeof(RSP_ATT_EXT_MTU));
	}else if(!bt_cmp(data, RECV_GATT_GET_SERVICE_1, sizeof(RECV_GATT_GET_SERVICE_1))){
		hci_send(RSP_GATT_GET_SERVICE_1, sizeof(RSP_GATT_GET_SERVICE_1));
	}else if(!bt_cmp(data, RECV_GATT_GET_SERVICE_2, sizeof(RECV_GATT_GET_SERVICE_2))){
		hci_send(RSP_GATT_GET_SERVICE_2, sizeof(RSP_GATT_GET_SERVICE_2));
	}else if(!bt_cmp(data, RECV_GATT_GET_SERVICE_3, sizeof(RECV_GATT_GET_SERVICE_3))){
		hci_send(RSP_GATT_GET_SERVICE_3, sizeof(RSP_GATT_GET_SERVICE_3));
	}else if(!bt_cmp(data, RECV_GATT_GET_SERVICE_4, sizeof(RECV_GATT_GET_SERVICE_4))){
		hci_send(RSP_GATT_GET_SERVICE_4, sizeof(RSP_GATT_GET_SERVICE_4));
	}else if(!bt_cmp(data, RECV_GATT_READ_CHAR, sizeof(RECV_GATT_READ_CHAR))){
		puts("GATT_READ_CHAR !");
		hci_send(RSP_GATT_READ_CHAR, sizeof(RSP_GATT_READ_CHAR));
	}else if(!bt_cmp(data, RECV_GATT_WRITE_CHAR, sizeof(RECV_GATT_WRITE_CHAR))){
		puts("GATT_WRITE !");
		hci_send(RSP_GATT_WRITE_CHAR, sizeof(RSP_GATT_WRITE_CHAR));
	}else if(!bt_cmp(data, RECV_GATT_READ_DESC, sizeof(RECV_GATT_READ_DESC))){
		puts("GATT_READ_DESC !");
		hci_send(RSP_GATT_READ_DESC, sizeof(RSP_GATT_READ_DESC));
	}
}

void recv_cb_client(char *data, int len)
{
	if(!bt_cmp(data, CMP_RESET, sizeof(CMP_RESET))){
		hci_send(SET_EVENT_MASK, sizeof(SET_EVENT_MASK));
	}else if(!bt_cmp(data, CMP_SET_EVENT_MASK, sizeof(CMP_SET_EVENT_MASK))){
		hci_send(LE_SET_EVENT_MASK, sizeof(LE_SET_EVENT_MASK));
	}else if(!bt_cmp(data, CMP_LE_SET_EVENT_MASK, sizeof(CMP_LE_SET_EVENT_MASK))){
		hci_send(LE_SET_ADV_PARAM, sizeof(LE_SET_ADV_PARAM));
	}else if(!bt_cmp(data, CMP_LE_SET_ADV_PARAM, sizeof(CMP_LE_SET_ADV_PARAM))){
		hci_send(LE_SET_ADV_DATA, sizeof(LE_SET_ADV_DATA));
	}else if(!bt_cmp(data, CMP_LE_SET_ADV_DATA, sizeof(CMP_LE_SET_ADV_DATA))){
		hci_send(LE_SET_SCAN_ENABLE, sizeof(LE_SET_SCAN_ENABLE));
	}else if(!bt_cmp(data, CMP_LE_SET_SCAN_ENABLE, sizeof(CMP_LE_SET_SCAN_ENABLE))){
		puts("Scan status changed !");
	}else if(!bt_cmp(data, RECV_L2CAP_PARAM_UPDATE_REQ, sizeof(RECV_L2CAP_PARAM_UPDATE_REQ))){
		puts("L2cap connection parameter updating !");
		hci_send(RSP_L2CAP_PARAM_UPDATE, sizeof(RSP_L2CAP_PARAM_UPDATE));
#define PEER_ADDRESS "\x00\xa1\x20\x66\xbf\x01"
	}else if(!bt_cmp(data, RECV_ADV_REPORT, sizeof(RECV_ADV_REPORT)) && !memcmp(&data[7], PEER_ADDRESS, 6)){
		puts("Connectiong to peer device !");
		hci_send(LE_SET_SCAN_DISABLE, sizeof(LE_SET_SCAN_DISABLE));
		hci_send("\x01\x0d\x20\x19\x20\x00\x18\x00\x00""\x00"PEER_ADDRESS"\x00\x40\x00\x80\x00\x01\x00\x00\x01\x00\x00\x00\x80", 29);
	}else if(!bt_cmp(data, RECV_LE_DISCONNECTED, sizeof(RECV_LE_DISCONNECTED))){
		puts("Device disconnected !");
	}else if(!bt_cmp(data, RECV_LE_CONNECTED, sizeof(RECV_LE_CONNECTED))){
		puts("Device connected !");
		hci_send(SEND_SMP_PAIRING_REQ, sizeof(SEND_SMP_PAIRING_REQ)); // SMP test
	}
}
int main(void)
{
	hci_init(USB_LOG_ALL, recv_cb_server);
	hci_send(CMD_RESET, sizeof(CMD_RESET));
	while(1)usleep(50000);
}
