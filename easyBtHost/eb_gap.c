#include <string.h>
#include "eb_gap.h"
#include "easyBle.h"

struct eb_gap_env{
    uint16_t conn_handle;
    bdaddr_t local_addr;
    uint8_t local_addr_type;
    bdaddr_t peer_addr;
    uint8_t peer_addr_type;
    uint8_t encrypted;
};

static struct eb_gap_env gap_env;

void eb_gap_init(void)
{
    ;
}

void eb_gap_connected_handler(uint16_t con_hdl, bdaddr_t addr, uint8_t addr_type)
{
    memcpy(&gap_env.peer_addr[0], &addr[0], 6);
    gap_env.peer_addr_type = addr_type;
    gap_env.conn_handle = con_hdl;
    gap_env.encrypted = false;
    eb_l2cap_init();
}

void eb_gap_disconnected_handler(uint16_t con_hdl, bdaddr_t addr, uint8_t addr_type)
{
    gap_env.peer_addr_type = -1;
}

uint8_t eb_gap_get_local_address(bdaddr_t addr)
{
    memcpy(&addr[0], &gap_env.local_addr[0], 6);
    return gap_env.local_addr_type;
}

uint8_t eb_gap_get_peer_address(bdaddr_t addr)
{
    memcpy(&addr[0], &gap_env.peer_addr[0], 6);
    return gap_env.peer_addr_type;
}

uint16_t eb_gap_get_conn_handle(void)
{
    return gap_env.conn_handle;
}

bool eb_gap_get_encrypted(uint16_t con_hdl)
{
    return gap_env.encrypted;
}

void eb_gap_set_encrypted(uint16_t con_hdl, bool encrypted)
{
    gap_env.encrypted = encrypted;
}

void eb_gap_set_random_address(bdaddr_t addr)
{
    uint8_t cmd[4+6] = {0x01, 0x05, 0x20, 0x06};
    memcpy(&cmd[4], (uint8_t*)&addr[0], sizeof(bdaddr_t));
    eb_h4_send(cmd, sizeof(cmd));
    gap_env.local_addr_type = 1;
    memcpy(&gap_env.local_addr[0], &addr[0], sizeof(bdaddr_t));
}

void eb_gap_adv_set_data(eb_adv_set_type_t type, uint8_t *data, uint8_t len)
{
    uint8_t cmd[4+32] = {0x01, type==EB_GAP_ADV_SET_DATA?0x08:0x09, 0x20, 0x20, 0x1F};
    memcpy(&cmd[5], data, len);
    eb_h4_send(cmd, sizeof(cmd));
}

void eb_gap_adv_set_param(uint16_t intv_min, uint16_t intv_max, eb_adv_type_t adv_type, 
                                    eb_adv_addr_type_t own_addr_type, eb_adv_addr_type_t peer_addr_type,
                                    bdaddr_t *peer_addr, uint8_t eb_adv_chn_map, eb_adv_filter_policy_t filter_policy)

{
    uint8_t cmd[4+15] = {0x01, 0x06, 0x20, 0x0F,
            intv_min&0xFF, intv_min>>8, intv_max&0xFF, intv_max>>8, adv_type, own_addr_type,
            peer_addr_type, 0,0,0,0,0,0, eb_adv_chn_map, filter_policy};
    if(peer_addr){ memcpy(&cmd[11], peer_addr, sizeof(bdaddr_t)); }
    eb_h4_send(cmd, sizeof(cmd));
}

void eb_gap_adv_enable(bool enable)
{
    uint8_t cmd[4+1] = {0x01, 0x0A, 0x20, 0x1, enable};
    eb_h4_send(cmd, sizeof(cmd));
}

void eb_gap_connect(bdaddr_t addr, uint8_t addr_type)
{
    uint8_t cmd[] = {
        0x01, 0x0D, 0x20, 0x19, 0x30, 0x00, 0x30, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x01,
        0x18, 0x00, 0x20, 0x00, 0,0, 200,0, 0,0, 0,0,
    };
    cmd[9] = addr_type;
    memcpy(&cmd[10], &addr[0], 6);
    eb_h4_send(cmd, sizeof(cmd));
}

void eb_gap_connect_cancel(void)
{
    uint8_t cmd[] = { 0x01, 0x0E, 0x20, 0x00 };
    eb_h4_send(cmd, sizeof(cmd));
}

void eb_gap_disconnect(uint16_t con_hdl, uint8_t reason)
{
    uint8_t cmd[] = { 0x01, 0x06, 0x04, 0x03, con_hdl&0xFF, con_hdl>>8, reason};
    eb_h4_send(cmd, sizeof(cmd));
}

void eb_gap_reset(void)
{
    uint8_t cmd[] = { 0x01, 0x03, 0x0C, 0x00 };
    eb_h4_send(cmd, sizeof(cmd));
}

void eb_gap_set_scan_param(uint8_t scan_type, uint16_t interval, uint16_t window,
                                        uint8_t addr_type, uint8_t filter_policy)
{
    uint8_t cmd[4+7] = {0x01, 0x0B, 0x20, 0x07, scan_type,
                        interval & 0xFF, interval >> 8, window & 0xFF, window >> 8,
                        addr_type, filter_policy };
    eb_h4_send(cmd, sizeof(cmd));
}

void eb_gap_scan_enable(bool enable, uint8_t filter_dup)
{
    uint8_t cmd[4+2] = {0x01, 0x0C, 0x20, 02, enable, filter_dup};
    eb_h4_send(cmd, sizeof(cmd));
}

