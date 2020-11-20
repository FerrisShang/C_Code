#include "stdlib.h"
#include "string.h"
#include "dfu_client.h"

#define DFU_FORSE_CREATE       0
#define BUF_SIZE               512   // Max size request in callback
#define DFU_FIFO_SIZE          1024
#define MAX_DFU_SEND_SIZE      495
#if DFU_FIFO_SIZE < BUF_SIZE + MAX_DFU_SEND_SIZE
#error "DFU_FIFO_SIZE MUST larget then BUF_SIZE + MAX_DFU_SEND_SIZE"
#endif

#define DFU_CTRL_CREATE        0x01
#define DFU_CTRL_SET_PRN       0x02
#define DFU_CTRL_CAL_CHECKSUM  0x03
#define DFU_CTRL_EXECUTE       0x04
#define DFU_CTRL_RESERVE       0x05
#define DFU_CTRL_SELECT        0x06
#define DFU_CTRL_RESPONSE      0x60

enum{
    DFU_ST_IDLE,
    DFU_ST_SEL,
    DFU_ST_PD_SEL,
    DFU_ST_SET_PRN,
    DFU_ST_PD_SET_PRN,
    DFU_ST_CREATE,
    DFU_ST_PD_CREATE,
    DFU_ST_WRITE,
    DFU_ST_GET_CRC,
    DFU_ST_PD_GET_CRC,
    DFU_ST_EXECUTE,
    DFU_ST_PD_EXECUTE,
};

#define DECODE32(u32, p) do{ \
    u32 = (p[0] << 0) + (p[1] << 8) + (p[2] << 16) + (p[3] << 24); \
    p += sizeof(uint32_t); \
}while(0)
#define ENCODE32(p, u) {*(p)++=(uint8_t)(u);*(p)++=(uint8_t)((u)>>8);*(p)++=(uint8_t)((u)>>16);*(p)++=(uint8_t)((u)>>24);}

/*** dfu fifo ***/
static uint8_t dfu_buffer[DFU_FIFO_SIZE];
static uint32_t dfu_sum_cnt;
static uint32_t fr, ra;
#define DFU_FIFO_LENGTH() ((ra-fr)&(DFU_FIFO_SIZE-1))
#define DFU_FIFO_AVAIL()  (DFU_FIFO_SIZE-DFU_FIFO_LENGTH()-1)
#define DFU_FIFO_CLEAR()  do{ fr = ra; dfu_sum_cnt = 0; } while(0)
#define DFU_FIFO_PUSH(data, len)                         \
    do{                                                  \
        uint32_t l = DFU_FIFO_SIZE-ra;                   \
        l = l < (len) ? l : (len);                       \
        memcpy(&dfu_buffer[ra], data, l);                \
        memcpy(dfu_buffer, (uint8_t*)(data)+l, (len)-l); \
        ra = (ra + (len)) & (DFU_FIFO_SIZE-1);           \
        dfu_sum_cnt += len;                              \
    }while(0);
#define DFU_FIFO_POP(buf, len)                           \
    do{                                                  \
        uint32_t l = DFU_FIFO_SIZE-fr;                   \
        l = l < (len) ? l : (len);                       \
        memcpy(buf, &dfu_buffer[fr], l);                 \
        memcpy((uint8_t*)(buf)+l, dfu_buffer, (len)-l);  \
        fr = (fr + (len)) & (DFU_FIFO_SIZE-1);           \
    }while(0);
/*** dfu fifo ***/

typedef struct {
    uint8_t state;
    uint8_t cur_type;
    uint8_t prn;
    uint8_t prn_cnt;
    int8_t  pkg_num;
    uint16_t send_len;
    uint32_t cmd_size;
    uint32_t cmd_offset;
    uint32_t cmd_crc;
    uint32_t data_size;
    uint32_t data_offset;
    uint32_t data_crc;
} om_dfu_clt_t;

static om_dfu_clt_t dfu;

static  uint32_t dfu_crc32(uint8_t const * p_data, uint32_t size, uint32_t const * p_crc);

// st_xxx -> fsm function
static void st_idle(void)
{
    dfu.state = DFU_ST_IDLE;
}
static void st_select(void)
{
    dfu.state = DFU_ST_SEL;
    dfu.prn_cnt = 0;
    DFU_FIFO_CLEAR();
    if(!dfu.pkg_num){ return; }
    uint8_t buf[] = { DFU_CTRL_SELECT, dfu.cur_type };
    dfu_client_gatt_send_cb(DFU_GATT_TYPE_CTRL, buf, sizeof(buf));
    dfu.state = DFU_ST_PD_SEL;
    dfu.pkg_num--;
}
static void st_set_prn(void)
{
    dfu.state = DFU_ST_SET_PRN;
    if(!dfu.pkg_num){ return; }
    uint8_t buf[] = { DFU_CTRL_SET_PRN, dfu.prn };
    dfu_client_gatt_send_cb(DFU_GATT_TYPE_CTRL, buf, sizeof(buf));
    dfu.state = DFU_ST_PD_SET_PRN;
    dfu.pkg_num--;
}
static void st_create(void)
{
    dfu.state = DFU_ST_CREATE;
    if(!dfu.pkg_num){ return; }
    uint32_t length;
    dfu_client_get_info_cb(dfu.cur_type, &length);
    uint8_t buf[2+sizeof(uint32_t)] = { DFU_CTRL_CREATE, dfu.cur_type }, *p = &buf[2];
    ENCODE32(p, length);
    if(dfu.cur_type == DFU_PKG_TYPE_CMD){
        dfu.cmd_size = length;
        dfu.cmd_offset = dfu.cmd_crc = 0;
    }else{
        dfu.data_size = length;
        dfu.data_offset = dfu.data_crc = 0;
    }
    dfu_client_gatt_send_cb(DFU_GATT_TYPE_CTRL, buf, sizeof(buf));
    dfu.state = DFU_ST_PD_CREATE;
    dfu.pkg_num--;
}
static void st_get_crc(void)
{
    dfu.state = DFU_ST_GET_CRC;
    if(!dfu.pkg_num){ return; }
    uint8_t buf[] = { DFU_CTRL_CAL_CHECKSUM };
    dfu_client_gatt_send_cb(DFU_GATT_TYPE_CTRL, buf, sizeof(buf));
    dfu.state = DFU_ST_PD_GET_CRC;
    dfu.pkg_num--;
}
static void st_write(void)
{
    dfu.state = DFU_ST_WRITE;
    if(!dfu.pkg_num){ return; }
    while(dfu.pkg_num){
        uint32_t buf_data_remain;
        if(dfu.cur_type == DFU_PKG_TYPE_CMD){
            buf_data_remain = dfu.cmd_size - dfu_sum_cnt;
        }else{
            buf_data_remain = dfu.data_size - dfu_sum_cnt;
        }
        if(buf_data_remain && DFU_FIFO_LENGTH() < dfu.send_len){
            uint32_t max_len, len;
            uint8_t buf[BUF_SIZE];
            max_len = buf_data_remain < BUF_SIZE ? buf_data_remain : BUF_SIZE;
            len = max_len;
            dfu_client_get_data_cb(dfu.cur_type, dfu_sum_cnt, &len, buf);
            buf_data_remain -= len;
            if(len == 0){
                return;
            }
            dfu_assert(max_len >= len);
            DFU_FIFO_PUSH(buf, len);
        }
        uint32_t send_len = DFU_FIFO_LENGTH() < dfu.send_len ? DFU_FIFO_LENGTH() : dfu.send_len;
        if((buf_data_remain == 0 && send_len) || send_len == dfu.send_len){
            uint8_t buf[MAX_DFU_SEND_SIZE];
            DFU_FIFO_POP(buf, send_len);
            dfu_client_gatt_send_cb(DFU_GATT_TYPE_DATA, buf, send_len);
            struct { uint32_t offset; uint32_t size; } prog;
            if(dfu.cur_type == DFU_PKG_TYPE_CMD){
                dfu.cmd_offset += send_len;
                dfu.cmd_crc = dfu_crc32(buf, send_len, &dfu.cmd_crc);
                prog.offset = dfu.cmd_offset;
                prog.size = dfu.cmd_size;
            }else{ // dfu.cur_type == DFU_PKG_TYPE_DATA
                dfu.data_offset += send_len;
                dfu.data_crc = dfu_crc32(buf, send_len, &dfu.data_crc);
                prog.offset = dfu.data_offset;
                prog.size = dfu.data_size;
            }
            dfu_client_evt_cb(DFU_EVT_PROG, &prog);
            dfu.pkg_num--;
            dfu.prn_cnt++;
            if((buf_data_remain == 0 && DFU_FIFO_LENGTH() == 0) || dfu.prn_cnt == dfu.prn){
                // All data transmited or rpn reached
                dfu.prn_cnt = 0;
                st_get_crc();
                return;
            }
        }
    }
}
static void st_execute(void)
{
    dfu.state = DFU_ST_EXECUTE;
    if(!dfu.pkg_num){ return; }
    uint8_t buf[] = { DFU_CTRL_EXECUTE };
    dfu_client_gatt_send_cb(DFU_GATT_TYPE_CTRL, buf, sizeof(buf));
    dfu.state = DFU_ST_PD_EXECUTE;
    dfu.pkg_num--;
}

static  uint32_t dfu_crc32(uint8_t const * p_data, uint32_t size, uint32_t const * p_crc)
{
    uint32_t crc;
    crc = (p_crc == NULL) ? 0xFFFFFFFF : ~(*p_crc);
    for (uint32_t i = 0; i < size; i++){
        crc = crc ^ p_data[i];
        for (uint32_t j = 8; j > 0; j--) crc = (crc >> 1) ^ (0xEDB88320U & ((crc & 1) ? 0xFFFFFFFF : 0));
    }
    return ~crc;
}
#if !DFU_FORSE_CREATE
static uint32_t get_crc_by_offset(uint8_t type, uint32_t offset)
{
    uint32_t c = 0, o = 0, max_len, len;
    uint8_t buf[BUF_SIZE];
    while(offset > o){
        max_len = offset - o;
        max_len = max_len < BUF_SIZE ? max_len : BUF_SIZE;
        len = max_len;
        dfu_client_get_data_cb(type, o, &len, buf);
        if(len == 0){
            return 0;
        }
        dfu_assert(max_len >= len);
        c = dfu_crc32(buf, len, &c);
        o += len;
    }
    return c;
}
#endif
// return 1->success, 0->failed, -1->ignore
static int check_crc(uint32_t offset, uint32_t crc)
{
    dfu_log("Check CRC(type:%d), O:%d<==>%d,C:0x%08X<==>0x%08X\n",
            dfu.cur_type,
            dfu.cur_type==DFU_PKG_TYPE_CMD?dfu.cmd_offset:dfu.data_offset,
            offset,
            dfu.cur_type==DFU_PKG_TYPE_CMD?dfu.cmd_crc:dfu.data_crc,
            crc
            );
    if(dfu.cur_type == DFU_PKG_TYPE_CMD){
        if(offset < dfu.cmd_offset){ return -1; }
        return offset == dfu.cmd_offset && crc == dfu.cmd_crc;
    }else{ // dfu.cur_type == DFU_PKG_TYPE_CMD
        if(offset < dfu.data_offset){ return -1; }
        return offset == dfu.data_offset && crc == dfu.data_crc;
    }
}

static void dfu_client_finish_cb(uint8_t state)
{
    dfu.state = DFU_ST_IDLE;
    dfu_client_evt_cb(DFU_EVT_END, &state);
}

void dfu_client_start(uint16_t mtu, uint8_t pkg_max_num, uint8_t prn)
{
    if(dfu.state != DFU_ST_IDLE){
        return;
    }
    dfu.state = DFU_ST_SEL;
    dfu.cur_type = DFU_PKG_TYPE_CMD;
    dfu.prn = prn;
    dfu.pkg_num = pkg_max_num;
    dfu.send_len = (mtu-3 < MAX_DFU_SEND_SIZE) ? (mtu-3) : MAX_DFU_SEND_SIZE;
    st_select();
}

void dfu_client_abort(void)
{
    if(dfu.state == DFU_ST_IDLE){
        return;
    }
    dfu.state = DFU_ST_IDLE;
    dfu.pkg_num = 0;
    dfu_client_finish_cb(DFU_UPDATE_ABORT);
}

void dfu_client_gatt_recv(uint8_t *data, uint32_t length)
{
    if(length >= 3 && data && data[0] == DFU_CTRL_RESPONSE){
        uint8_t opcode = data[1], st = dfu.state, res = data[2];
        if(res != DFU_SUCCESS){
            dfu_log("Slave report error: 0x%X\n", res);
            dfu_client_finish_cb(res);
            return;
        }
        if(opcode == DFU_CTRL_CREATE && st == DFU_ST_PD_CREATE){
            st_write();
        }else if(opcode == DFU_CTRL_SET_PRN && st == DFU_ST_PD_SET_PRN){
            st_create();
        }else if(opcode == DFU_CTRL_CAL_CHECKSUM && st == DFU_ST_PD_GET_CRC){
            uint8_t *p = &data[3];
            uint32_t offset, crc;
            DECODE32(offset, p);
            DECODE32(crc, p);
            int res = check_crc(offset, crc);
            if(res > 0){
                if((dfu.cur_type == DFU_PKG_TYPE_CMD && offset == dfu.cmd_size) ||
                        (dfu.cur_type == DFU_PKG_TYPE_DATA && offset == dfu.data_size)){
                    st_execute();
                }else{
                    st_write();
                }
            }else if(res == 0){
                dfu_client_finish_cb(DFU_CRC_NOT_MATCH);
            }
        }else if(opcode == DFU_CTRL_EXECUTE && st == DFU_ST_PD_EXECUTE){
            if(dfu.cur_type == DFU_PKG_TYPE_CMD){
                dfu.cur_type = DFU_PKG_TYPE_DATA;
                st_select(); // cmd obj finished
            }else{
                st_idle(); // cmd & data all finished
                dfu_client_finish_cb(res);
            }
        }else if(opcode == DFU_CTRL_SELECT && st == DFU_ST_PD_SEL){
            if(length >= 15){
#if !DFU_FORSE_CREATE
                uint32_t size, offset, crc;
                uint8_t *p = &data[3];
                DECODE32(size, p);
                DECODE32(offset, p);
                DECODE32(crc, p);
                if(size > 0 && offset > 0){
                    uint32_t get_length, get_crc;
                    dfu_client_get_info_cb(dfu.cur_type, &get_length);
                    if(get_length == size){
                        get_crc = get_crc_by_offset(dfu.cur_type, offset);
                        if(get_crc == crc){
                            // crc match, transport data from offset
                            dfu_log("Transmit data from offset 0x%08X\n", offset);
                            st_write();
                            return;
                        }
                    }
                }
#endif
                // create a new one
                st_set_prn();
            }else{
                dfu_log("Unexcepted data received.\n");
            }
        }
    }else{
        dfu_log("Unexcepted data received.\n");
    }
}

void dfu_client_can_send(uint8_t pkg_num)
{
    dfu.pkg_num = pkg_num;
    // DFU_ST_WRITE
    if(dfu.state == DFU_ST_WRITE){
        st_write();
    }else if(dfu.state == DFU_ST_SEL){
        st_select();
    }else if(dfu.state == DFU_ST_SET_PRN){
        st_set_prn();
    }else if(dfu.state == DFU_ST_CREATE){
        st_create();
    }else if(dfu.state == DFU_ST_GET_CRC){
        st_get_crc();
    }else if(dfu.state == DFU_ST_EXECUTE){
        st_execute();
    }
}

#ifndef __WEAK
#define __WEAK __attribute__((weak))
#endif
__WEAK void dfu_client_evt_cb(uint8_t evt, void* param)
{
    if(evt == DFU_EVT_PROG){
        dfu_log("dfu prog: %d/%d\n", *(uint32_t*)param, *((uint32_t*)param+1));
    }else{
        dfu_log("dfu finished: %d\n", *(uint8_t*)param);
    }
}
__WEAK void dfu_client_get_info_cb(uint8_t pkg_type, uint32_t *length){}
__WEAK void dfu_client_get_data_cb(uint8_t pkg_type, uint32_t offset, uint32_t *max_len, uint8_t *data){}
__WEAK void dfu_client_gatt_send_cb(uint8_t gatt_type, uint8_t *data, uint32_t length){}

