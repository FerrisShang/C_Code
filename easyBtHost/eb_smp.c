#include <string.h>
#include <stdlib.h>
#include "easyBle.h"
#include "eb_smp.h"
#include "eb_gap.h"

#define SMP_PAIRING_RSP_DATA 0x02,0x03,0x00,0x01,0x10,0x01,0x01
static const uint8_t tk[16] = {0};

static uint8_t smp_encrypt_buf[16];
static uint8_t smp_local_rand_value[16];

static uint8_t *smp_iat = &smp_encrypt_buf[0];
static uint8_t *smp_rat = &smp_encrypt_buf[1];
static uint8_t *smp_pairing_req = &smp_encrypt_buf[2];
static uint8_t *smp_pairing_rsp = &smp_encrypt_buf[9];

enum{
    EM_SMP_ST_IDLE = 0,
    EM_SMP_ST_PAIRING_REQ,
    EM_SMP_ST_PAIRING_RSP,
    EM_SMP_ST_CONFIRM_REQ,
    EM_SMP_ST_CONFIRM_RSP,
    EM_SMP_ST_RAND_REQ,
    EM_SMP_ST_RAND_RSP,
    EM_SMP_ST_LTK_REQ,
    EM_SMP_ST_LTK_RSP,
    EM_SMP_ST_ENC_RSP,
};

enum{
    EM_SMP_ENC_ST_IDLE = 0,
    EM_SMP_ENC_ST_CONFIRM_CAL1,
    EM_SMP_ENC_ST_CONFIRM_CAL2,
    EM_SMP_ENC_ST_CONFIRM_PENDING,
    EM_SMP_ENC_ST_STK,
};

struct eb_smp_env{
    uint16_t conn_hd;
    uint8_t role;
    uint8_t smp_state;
    uint8_t smp_encrypt_state;
};


static struct eb_smp_env smp_env;

void eb_smp_init(void)
{
}

void eb_smp_connected_handler(uint16_t con_hdl, bdaddr_t addr, uint8_t addr_type, uint8_t role)
{
    smp_env.role = role;
    smp_env.conn_hd = con_hdl;
}

void eb_smp_disconnected_handler(uint16_t con_hdl, bdaddr_t addr, uint8_t addr_type)
{
    smp_env.conn_hd = 0xFFFF;
}

static void smp_send_confirm(uint16_t conn_hd, uint8_t *confirm_value)
{
    uint8_t cmd[9+17] = {0x02, conn_hd&0xFF, conn_hd>>8, 0x15, 0x00, 0x11, 0x00, 0x06, 0x00, 0x03};
    memcpy(&cmd[10], confirm_value, 16);
    eb_h4_send(cmd, sizeof(cmd));
}

static void smp_enc_cal_cb(uint8_t *encrypted)
{
    if(smp_env.smp_encrypt_state == EM_SMP_ENC_ST_CONFIRM_CAL1){
        bdaddr_t ia ,ra; int i;
        eb_gap_get_peer_address(ia);
        eb_gap_get_local_address(ra);
        for(i=0;i<6;i++){
            encrypted[i+0] ^= ra[i];
            encrypted[i+6] ^= ia[i];
        }
        em_hci_encrypt(encrypted, (uint8_t*)tk, smp_enc_cal_cb);
        smp_env.smp_encrypt_state = EM_SMP_ENC_ST_CONFIRM_CAL2;
    }else if(smp_env.smp_encrypt_state == EM_SMP_ENC_ST_CONFIRM_CAL2){
        if(smp_env.smp_state == EM_SMP_ST_CONFIRM_REQ){
            smp_send_confirm(smp_env.conn_hd, encrypted);
            smp_env.smp_encrypt_state = EM_SMP_ENC_ST_IDLE;
            smp_env.smp_state = EM_SMP_ST_CONFIRM_RSP;
        }else{
            memcpy(smp_encrypt_buf, encrypted, 16);
            smp_env.smp_encrypt_state = EM_SMP_ENC_ST_CONFIRM_PENDING;
        }
    }else if(smp_env.smp_encrypt_state == EM_SMP_ENC_ST_STK){
        smp_env.smp_state = EM_SMP_ST_LTK_RSP;
        smp_env.smp_encrypt_state = EM_SMP_ENC_ST_IDLE;
        uint8_t cmd[4+18] = {0x01, 0x1A, 0x20, 0x12, smp_env.conn_hd&0xFF, smp_env.conn_hd>>8};
        memcpy(&cmd[6], encrypted, 16);
        eb_h4_send(cmd, sizeof(cmd));
    }
}

static void smp_cal_confirm(void)
{
    int i;
    bdaddr_t addr;
    *smp_iat = eb_gap_get_peer_address(&addr[0]);
    *smp_rat = eb_gap_get_local_address(&addr[0]);
    for(i=0;i<16;i++){ smp_encrypt_buf[i] ^= smp_local_rand_value[i]; }
    em_hci_encrypt(smp_encrypt_buf, (uint8_t*)tk, smp_enc_cal_cb);
    smp_env.smp_encrypt_state = EM_SMP_ENC_ST_CONFIRM_CAL1;
}

void eb_smp_handler(uint8_t *data, uint16_t len)
{
    uint16_t conn_hd = (data[1] + (data[2]<<8)) & 0x0FFF;
    uint8_t code = data[9];
    switch(code){
        case 0x01:{ // Pairing Request
            smp_env.smp_state = EM_SMP_ST_PAIRING_REQ;
            // response request
            uint8_t cmd[9+7] = {0x02, conn_hd&0xFF, conn_hd>>8, 0x0b, 0x00, 0x07, 0x00, 0x06, 0x00,
                                SMP_PAIRING_RSP_DATA};
            eb_h4_send(cmd, sizeof(cmd));
            smp_env.smp_state = EM_SMP_ST_PAIRING_RSP;

            // calulate confirm value
            int i;
            for(i=0;i<16;i++){ smp_local_rand_value[i] = rand()&0xFF; }
            memcpy(smp_pairing_req, &data[9], 7);
            memcpy(smp_pairing_rsp, &cmd[9], 7);
            smp_cal_confirm();
            break;}
        case 0x03:{ // Pairing Confirm
            smp_env.smp_state = EM_SMP_ST_CONFIRM_REQ;
            if(smp_env.smp_encrypt_state == EM_SMP_ENC_ST_CONFIRM_PENDING){
                smp_env.smp_encrypt_state = EM_SMP_ENC_ST_IDLE;
                smp_send_confirm(conn_hd, smp_encrypt_buf);
                smp_env.smp_state = EM_SMP_ST_CONFIRM_RSP;
            }
            break;}
        case 0x04:{ // Pairing Random
            smp_env.smp_state = EM_SMP_ST_RAND_REQ;
            uint8_t cmd[9+17] = {0x02, conn_hd&0xFF, conn_hd>>8, 0x15, 0x00, 0x11, 0x00, 0x06, 0x00, 0x04};
            memcpy(&cmd[10], smp_local_rand_value, 16);
            eb_h4_send(cmd, sizeof(cmd));
            smp_env.smp_state = EM_SMP_ST_RAND_RSP;
            memcpy(&smp_encrypt_buf[8], &cmd[10], 8);
            memcpy(&smp_encrypt_buf[0], &data[10], 8);
            break;}
        default:
            break;
    }
}

bool eb_smp_get_ltk(void)
{
    if(smp_env.smp_state == EM_SMP_ST_RAND_RSP){
        smp_env.smp_state = EM_SMP_ST_LTK_REQ;
        smp_env.smp_encrypt_state = EM_SMP_ENC_ST_STK;
        em_hci_encrypt(smp_encrypt_buf, (uint8_t*)tk, smp_enc_cal_cb);
        return true;
    }else{
        return false;
    }
}

void eb_smp_encrpyt_change(void)
{
    if(smp_env.smp_state == EM_SMP_ST_LTK_RSP){
        smp_env.smp_state = EM_SMP_ST_ENC_RSP;
        uint8_t enc_info[9+17] = {0x02, smp_env.conn_hd&0xFF, smp_env.conn_hd>>8, 0x15, 0x00, 0x11, 0x00, 0x06, 0x00, 0x06};
        uint8_t enc_master[9+11] = {0x02, smp_env.conn_hd&0xFF, smp_env.conn_hd>>8, 0x0f, 0x00, 0x0b, 0x00, 0x06, 0x00, 0x07};

        eb_event_t evt = { EB_EVT_GAP_ENC_REQUEST };
        evt.gap.enc_request.handle = smp_env.conn_hd;
        evt.gap.enc_request.ediv = &enc_master[10];
        evt.gap.enc_request.random = &enc_master[12];
        evt.gap.enc_request.ltk = &enc_info[10];
        eb_event(&evt);
        eb_h4_send(enc_info, sizeof(enc_info));
        eb_h4_send(enc_master, sizeof(enc_master));
        smp_env.smp_state = EM_SMP_ST_IDLE;
    }
}

void eb_smp_auth(uint16_t conn_hd)
{
    if(smp_env.role){
        uint8_t cmd[9+2] = {0x02, conn_hd&0xFF, conn_hd>>8, 0x6, 0x00, 0x2, 0x00, 0x06, 0x00, 0x0B, 0x01};
        eb_h4_send(cmd, sizeof(cmd));
    }else{
        // TODO: pairing request
    }
}
