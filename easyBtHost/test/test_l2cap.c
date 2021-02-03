#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "eb_l2cap.h"
#define DUMP(d,l) do{int i;for(i=0;i<l;i++){printf("%02X ", ((uint8_t*)d)[i]);} printf("\n");}while(0)

void l2cap_packet_inc(void);
void l2cap_packet_dec(int num);
void l2cap_packet_comp(int num);

int usb_hci_send(uint8_t* data, int len)
{
    DUMP(data, len);
}

void eb_h4_send(uint8_t *data, int len)
{
    if(data[0] == 2){
        eb_l2cap_send(data, len);
    }else{
        usb_hci_send(data, len);
    }
}
int main(void)
{
    uint8_t long_data[] = {
        0x02, 0xee,0x0c, 107,0, 103,0, 0x04,0, 0x52, 0xaa,0xbb,
        0,1,2,3,4,5,6,7,8,9,  1,1,2,3,4,5,6,7,8,9,  2,1,2,3,4,5,6,7,8,9,  3,1,2,3,4,5,6,7,8,9,  4,1,2,3,4,5,6,7,8,9,
        5,1,2,3,4,5,6,7,8,9,  6,1,2,3,4,5,6,7,8,9,  7,1,2,3,4,5,6,7,8,9,  8,1,2,3,4,5,6,7,8,9,  9,1,2,3,4,5,6,7,8,9,
    };
    printf(" --> Send a lot of data\n");
    int i;
    for(i=0;i<0x1000;i++){
        eb_l2cap_send(long_data, sizeof(long_data));
        l2cap_packet_comp(4);
    }
    printf(" --> Send a lot of data Done\n");

    printf(" --> Send update conn param\n");
    eb_l2cap_update_conn_param(0x1234, 0x0020, 0x0040, 0x0000, 0x0100);
    printf(" --> Send long att data\n");
    eb_l2cap_send(long_data, sizeof(long_data));
    eb_l2cap_send(long_data, sizeof(long_data));
    printf(" --> Send long att data, stuck\n");
    eb_l2cap_send(long_data, sizeof(long_data));
    printf(" --> now host acl count = %d\n", l2cap_packet_num());
    printf(" --> packet complete, send resume\n");
    l2cap_packet_comp(9);
    printf(" --> now host acl count = %d\n", l2cap_packet_num());

    printf(" --> Recv Update ACL data\n");
    eb_l2cap_handler((uint8_t*)"\x02\xee\x0c\x04\x00\x00\x00\x01\x00\x12\xFF", 11);
    printf(" --> Recv Unknown ACL data\n");
    eb_l2cap_handler((uint8_t*)"\x02\xee\x0c\x04\x00\x00\x00\x01\x00\xBB\xFF", 11);
    return 0;
}
