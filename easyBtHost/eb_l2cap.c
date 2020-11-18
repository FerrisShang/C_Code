#include "easyBle.h"
#include "eb_l2cap.h"

struct eb_l2cap_env{
    uint8_t packets_num;
};

static struct eb_l2cap_env l2cap_env;


void eb_l2cap_init(void)
{
    l2cap_env.packets_num = 0;
}

void eb_l2cap_update_conn_param(uint16_t conn_hd,
        uint16_t intv_min, uint16_t intv_max, uint16_t latency, uint16_t timeout)
{
    uint8_t cmd[9+12] = {0x02, conn_hd&0xFF, conn_hd>>8, 0x10, 0x00, 0x0c, 0x00, 0x05, 0x00, 0x12,0xff,0x08,0x00,
        intv_min&0xFF, intv_min>>8, intv_max&0xFF, intv_max>>8, latency&0xFF, latency>>8, timeout&0xFF, timeout>>8};
    eb_h4_send(cmd, sizeof(cmd));
}
void eb_l2cap_handler(uint8_t *data, uint16_t len)
{
    uint16_t conn_hd = (data[1] + (data[2]<<8)) & 0x0FFF;
    uint8_t opcode = data[9];
    uint8_t id = data[10];
    switch(opcode){
        case 0x12:{ // Connection parameter update request
            uint8_t cmd[9+6] = {0x02, conn_hd&0xFF, conn_hd>>8, 0x0A, 0x00, 0x06, 0x00, 0x05, 0x00, 0x13, id, 0x02, 0x00, 0x01, 0x00};
            eb_h4_send(cmd, sizeof(cmd));
        }   break;
        default:{
            uint8_t cmd[9+6] = {0x02, conn_hd&0xFF, conn_hd>>8, 0x0A, 0x00, 0x06, 0x00, 0x05, 0x00, 0x01, id, 0x02, 0x00, 0x00, 0x00};
            eb_h4_send(cmd, sizeof(cmd));
        }   break;
    }
}

void l2cap_packet_inc(void)
{
    l2cap_env.packets_num++;
}
void l2cap_packet_dec(int num)
{
    l2cap_env.packets_num -= num;
}
int l2cap_packet_num(void)
{
    return l2cap_env.packets_num;
}
