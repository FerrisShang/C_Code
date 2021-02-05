#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <pthread.h>
#include "eb_hci.h"

void dump(uint8_t* data, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

void eb_h4_send(uint8_t* data, int len)
{
    printf("Send: ");
    dump(data, len);
}

void usb_hci_send(uint8_t *data, int len)
{
    eb_h4_send(data, len);
}


int main(int argc, char* argv[])
{
    int res;
    srand(0);
    void encrypt_cb(uint8_t* data);

    // encrypt test
    res = em_hci_encrypt( (uint8_t*)"\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33", (uint8_t*)"\x77\x77\x77\x77\x77\x77\x77\x77\x77\x77\x77\x77\x77\x77\x77\x77", encrypt_cb);
    printf("em_hci_encrypt res = %d\n", res);
    res = em_hci_encrypt( (uint8_t*)"\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33", (uint8_t*)"\x77\x77\x77\x77\x77\x77\x77\x77\x77\x77\x77\x77\x77\x77\x77\x77", encrypt_cb);
    printf("em_hci_encrypt res = %d\n", res);
    eb_hci_handler((uint8_t*)"\x04\x0e\x13\x01\x17\x20\x10", 22);
    res = em_hci_encrypt( (uint8_t*)"\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33", (uint8_t*)"\x77\x77\x77\x77\x77\x77\x77\x77\x77\x77\x77\x77\x77\x77\x77\x77", encrypt_cb);
    printf("em_hci_encrypt res = %d\n", res);
    eb_hci_handler((uint8_t*)"\x04\x0e\x13\x01\x17\x20\x10", 22);
    // evt test
    printf("->* Encrpyt complete\n");
    eb_hci_handler((uint8_t*)"\x04\x0e\x13\x01\x17\x20\x10", 22);
    printf("->* Reset complete\n");
    eb_hci_handler((uint8_t*)"\x04\x0e\x04\x05\x03\x0c\x00", 7);
    printf("->* Encrypte change complete\n");
    eb_hci_handler((uint8_t*)"\x04\x08\x04\x05\x03\x0c\x01", 7);
    printf("->* L2cap_packet complete\n");
    eb_hci_handler((uint8_t*)"\x04\x13\x04\x01\x0e\x0c\x01", 7);
    printf("->* Disconnect complete\n");
    eb_hci_handler((uint8_t*)"\x04\x05\x04\x00\x0e\x0c\x16", 7);
    printf("->* Connect complete\n");
    eb_hci_handler((uint8_t*)"\x04\x3e\x13\x01\x00\x0d\x1e\x01\x01\xaa\xaa\xaa\xaa\xaa\xaa\x20\x00\x00\x00\x00\x01\x00", 22);
    printf("->* Adv report received\n");
    eb_hci_handler((uint8_t*)"\x04\x3e\x13\x02\x01\x0d\x1e\x01\x01\xaa\xaa\xaa\xaa\xaa\xaa\x20\x00\x00\x00\x00\x01\x00", 22);
    printf("->* Parameter updated received\n");
    eb_hci_handler((uint8_t*)"\x04\x3e\x13\x03\x01\x0d\x1e\x01\x01\xaa\xaa\xaa\xaa\xaa\xaa\x20\x00\x00\x00\x00\x01\x00", 22);
    printf("->* LTK request received\n");
    eb_hci_handler((uint8_t*)"\x04\x3e\x0d\x05\x0d\x1e\x00\x00\x00\x00\x00\x00\x00\x00\xed\x1c", 16);


    printf("->* ATT received len=29\n");
    eb_hci_handler((uint8_t*)"\x02\x02\x20\x18\x00\x14\x00\x04\x00\x11\x06\x10\x00\x21\x00\x12\x18\x22\x00\x29\x00\xe7\xfe\x2a\x00\x31\x00\x59\xfe", 29);
    printf("->* SMP received len=16\n");
    eb_hci_handler((uint8_t*)"\x02\x02\x20\x0b\x00\x07\x00\x06\x00\x01\x04\x00\x2d\x10\x0f\x0f", 16);

    printf("->* Long ATT received len=209\n");
    eb_hci_handler((uint8_t*)"\x02\x0a\x20\x7b\x00\xc8\x00\x04\x00\x52\x34\x00\x3a\x21\x06\x20\xfe\xf7\xe5\xfd\xce\x49\xaa\x22\x06\x98\x70\x23\xcd\xe9\x00\x21\x02\x90\x1a\x22\x3a\x21\xd0\x20\xfe\xf7\xd9\xfd\xc9\x49\x4e\x22\x06\x98\x0e\x23\xcd\xe9\x00\x21\x02\x90\x38\x22\xdc\x21\x5c\x20\xfe\xf7\xcd\xfd\x16\xf0\xa6\xfe\x06\x98\x00\xeb\x40\x00\xc0\x00\x18\x23\xf0\x22\x01\x46\x05\x90\x00\x20\x14\xf0\xa8\xf9\x18\x20\x02\xf0\xcb\xf9\x06\x98\x40\x1c\xc0\xb2\x06\x90\x06\x98\x0a\x28\xff\xf6\x5b\xaf\x5f\xe5\x00\x20\x06\x90\x3e\xe0\xfd\xf7\x05\xfd", 128);
    eb_hci_handler((uint8_t*)"\x02\x0a\x10\x51\x00\xb5\x49\x4f\xf4\x80\x72\x06\x98\xf0\x23\xcd\xe9\x00\x21\x02\x90\x1a\x46\x00\x21\x08\x46\xfe\xf7\xa2\xfd\xa0\x48\x00\x78\x60\xb1\xae\x49\x11\x22\x06\x98\x10\x23\xcd\xe9\x00\x21\x02\x90\x4f\x22\x70\x21\x50\x20\xfe\xf7\x93\xfd\x0b\xe0\xa8\x49\x10\x22\x06\x98\x13\x23\xcd\xe9\x00\x21\x02\x90\x27\x22\x6e\x21\x64\x20\xfe\xf7\x86", 86);


    int i;
    printf("->* 5 reest send. command pending.\n");
    for(i=0;i<5;i++){
        eb_hci_send((uint8_t*)"\x01\x03\x0c\x00", 4);
    }
    printf("->* command complete received. send continue.\n");
    for(i=0;i<5;i++){
        eb_hci_handler((uint8_t*)"\x04\x0e\x04\x05\x03\x0c\x00", 7);
    }
    printf("PASS!\n");
    return 0;
}

bool eb_smp_get_ltk(void) { return 0; }
void encrypt_cb(uint8_t* data){printf("%s@%d\n", __func__, __LINE__);}
void eb_gap_disconnected_handler(uint16_t con_hdl, bdaddr_t addr, uint8_t addr_type) {printf("%s@%d\n", __func__, __LINE__);}
void eb_smp_encrpyt_change(void) {printf("%s@%d\n", __func__, __LINE__);}
void eb_gap_set_encrypted(uint16_t con_hdl, bool encrypted) {printf("%s@%d\n", __func__, __LINE__);}
void l2cap_packet_comp(int num) {printf("%s@%d\n", __func__, __LINE__);}
void eb_gap_connected_handler(uint16_t con_hdl, bdaddr_t addr, uint8_t addr_type) {printf("%s@%d\n", __func__, __LINE__);}
void eb_smp_disconnected_handler(uint16_t con_hdl, bdaddr_t addr, uint8_t addr_type) {printf("%s@%d\n", __func__, __LINE__);}
void eb_smp_connected_handler(uint16_t con_hdl, bdaddr_t addr, uint8_t addr_type, uint8_t role) {printf("%s@%d\n", __func__, __LINE__);}
void eb_l2cap_handler(uint8_t* data, uint16_t len) {printf("%s@%d len=%d\n", __func__, __LINE__, len);}
void eb_att_handler(uint8_t* data, uint16_t len) {printf("%s@%d len=%d\n", __func__, __LINE__, len);}
void eb_smp_handler(uint8_t* data, uint16_t len) {printf("%s@%d len=%d\n", __func__, __LINE__, len);}

void eb_event(eb_event_t* param)
{
    printf("%s@%d evt_id=0x%0X\n", __func__, __LINE__, param->evt_id);
}
