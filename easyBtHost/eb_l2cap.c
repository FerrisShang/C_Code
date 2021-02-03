#include <assert.h>
#include <string.h>
#include "easyBle.h"
#include "eb_l2cap.h"

struct eb_l2cap_env{
    int8_t host_pkt_num;
    int8_t ctrl_pkt_num;
};

static struct eb_l2cap_env l2cap_env;


void eb_l2cap_init(void)
{
    l2cap_env.host_pkt_num = 0;
    l2cap_env.ctrl_pkt_num = 0;
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
    l2cap_env.host_pkt_num++;
}
void l2cap_packet_dec(int num)
{
    l2cap_env.host_pkt_num -= num;
}
int l2cap_packet_num(void)
{
    return l2cap_env.host_pkt_num;
}
static void l2cap_buffer_send(void);
void l2cap_packet_comp(int num)
{
    l2cap_env.ctrl_pkt_num -= num;
    l2cap_buffer_send();
}

/*** l2cap fifo ***/
#define L2CAP_BUFFER_SIZE (2048*10)
typedef uint16_t l2cap_size_t;
static uint8_t l2cap_buffer[L2CAP_BUFFER_SIZE];
static l2cap_size_t fr, ra;
#define L2CAP_FIFO_LENGTH() ((ra-fr)&(L2CAP_BUFFER_SIZE-1))
#define L2CAP_FIFO_AVAIL()  (L2CAP_BUFFER_SIZE-L2CAP_FIFO_LENGTH()-1)
#define L2CAP_FIFO_CLEAR()  do{ fr = ra; } while(0)
#define L2CAP_FIFO_PUSH(data, len)                         \
    do{                                                    \
        assert(L2CAP_FIFO_AVAIL()>=len);                   \
        l2cap_size_t l = L2CAP_BUFFER_SIZE-ra;             \
        l = l < (len) ? l : (len);                         \
        memcpy(&l2cap_buffer[ra], data, l);                \
        memcpy(l2cap_buffer, (uint8_t*)(data)+l, (len)-l); \
        ra = (ra + (len)) & (L2CAP_BUFFER_SIZE-1);         \
    }while(0);
#define L2CAP_FIFO_POP(buf, len)                           \
    do{                                                    \
        assert(L2CAP_FIFO_LENGTH()>=len);                  \
        l2cap_size_t l = L2CAP_BUFFER_SIZE-fr;             \
        l = l < (len) ? l : (len);                         \
        memcpy(buf, &l2cap_buffer[fr], l);                 \
        memcpy((uint8_t*)(buf)+l, l2cap_buffer, (len)-l);  \
        fr = (fr + (len)) & (L2CAP_BUFFER_SIZE-1);         \
    }while(0);
/*** l2cap fifo ***/
#define END_FLAG 0x80000000
#define MAX_PKT_NUM  8
#define ACL_MAX_SIZE 27
int usb_hci_send(uint8_t* data, int len);
uint16_t eb_gap_get_mtu(void);
static void l2cap_buffer_send(void)
{
    uint32_t len;
    while(l2cap_env.ctrl_pkt_num < MAX_PKT_NUM && L2CAP_FIFO_LENGTH() >= sizeof(len)){
        L2CAP_FIFO_POP(&len, sizeof(len));
        if(len & END_FLAG){
            l2cap_packet_dec(1);
        }
        len = len & ~END_FLAG;
        assert(L2CAP_FIFO_LENGTH() >= len);
        uint8_t buf[len];
        L2CAP_FIFO_POP(buf, len);
        usb_hci_send(buf, len);
        l2cap_env.ctrl_pkt_num++;
    }
}
void eb_l2cap_send(uint8_t *data, uint32_t len)
{
    uint32_t push_len;
    if(len - 5 <= ACL_MAX_SIZE ){
        push_len = len | END_FLAG;
        L2CAP_FIFO_PUSH(&push_len, sizeof(push_len));
        L2CAP_FIFO_PUSH(data, len);
    }else{
        uint8_t buf[ACL_MAX_SIZE + 5];
        uint32_t i;
        memcpy(buf, data, 5);
        for(i=5;i<len;i+=ACL_MAX_SIZE){
            if(i == 5 + ACL_MAX_SIZE){
                buf[2] |= 0x10;
            }
            if(i + ACL_MAX_SIZE >= len){ // last one
                push_len = (len - i) + 5;
                uint32_t data_len = (len - i);
                buf[3] = data_len & 0xFF;
                buf[4] = data_len >> 8;
                push_len |= END_FLAG;
                L2CAP_FIFO_PUSH(&push_len, sizeof(push_len));
                push_len &= ~END_FLAG;
                memcpy(&buf[5], &data[i], data_len);
                L2CAP_FIFO_PUSH(buf, push_len);
            }else{
                push_len = ACL_MAX_SIZE + 5;
                buf[3] = ACL_MAX_SIZE & 0xFF;
                buf[4] = ACL_MAX_SIZE >> 8;
                L2CAP_FIFO_PUSH(&push_len, sizeof(push_len));
                memcpy(&buf[5], &data[i], ACL_MAX_SIZE);
                L2CAP_FIFO_PUSH(buf, ACL_MAX_SIZE + 5);
            }
        }
    }
    l2cap_packet_inc();
    l2cap_buffer_send();
}

