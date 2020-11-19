#ifndef __EB_GAP_H__
#define __EB_GAP_H__

#include "eb_config.h"

enum{
    EB_EVT_GAP_RESET = (EB_GAP_ID << 8),
    EB_EVT_GAP_CONNECTED,
    EB_EVT_GAP_DISCONNECTED,
    EB_EVT_GAP_ENCRYPTED,
    EB_EVT_GAP_ENC_REQUEST,
    EB_EVT_GAP_PARAM_UPDATED,
    EB_EVT_GAP_LTK_REQUEST,
    EB_EVT_GAP_ADV_REPORT,
    EB_EVT_GAP_TX_COMPLETE,
    EB_EVT_GAP_MTU_UPDATE,
};

typedef enum {
    EB_GAP_REPORT_IND = 0,
    EB_GAP_REPORT_DIRECT_IND,
    EB_GAP_REPORT_SCAN_IND,
    EB_GAP_REPORT_NONCONN_IND,
    EB_GAP_REPORT_SCAN_RSP,
}eb_report_type_t;

typedef enum {
    EB_GAP_ADV_SET_DATA,
    EB_GAP_ADV_SET_SCAN_RSP,
}eb_adv_set_type_t;

typedef enum {
    EB_GAP_ADV_IND = 0,
    EB_GAP_ADV_DIRECT_HIGH_IND,
    EB_GAP_ADV_SCAN_IND,
    EB_GAP_ADV_NONCONN_IND,
    EB_GAP_ADV_DIRECT_LOW_IND,
}eb_adv_type_t;

typedef enum {
    EB_ADV_ADDR_TYPE_PUBLIC = 0,
    EB_ADV_ADDR_TYPE_RANDOM,
    EB_ADV_ADDR_TYPE_RPA_OR_PUBLIC,
    EB_ADV_ADDR_TYPE_RPA_OR_RANDOM,
}eb_adv_addr_type_t;


typedef enum {
    EB_ADV_FILTER_DEFAULT = 0,
    EB_ADV_FILTER_SCAN,
    EB_ADV_FILTER_CONN,
    EB_ADV_FILTER_ALL,
}eb_adv_filter_policy_t;

typedef struct {
    uint8_t status;
    uint8_t sca;
    uint16_t handle;
    uint8_t role;
    uint8_t peer_addr_type;
    bdaddr_t peer_addr;
    uint16_t interval;
    uint16_t latency;
    uint16_t timeout;
}eb_gap_connected_t;

typedef struct {
    uint8_t status;
    uint8_t reason;
    uint16_t handle;
}eb_gap_disconnected_t;

typedef struct {
    uint8_t status;
    uint16_t handle;
}eb_gap_encrypt_change_t;

typedef struct {
    uint16_t handle;
    uint8_t *ediv;
    uint8_t *random;
    uint8_t *ltk;
}eb_gap_enc_request_t;

typedef struct {
    uint16_t handle;
    uint8_t *ediv;
    uint8_t *random;
    uint8_t ltk_found;
    uint8_t *ltk;
}eb_gap_ltk_request_t;

typedef struct {
    eb_report_type_t type;
    eb_adv_addr_type_t addr_type;
    bdaddr_t addr;
    int8_t rssi;
    uint8_t length;
    uint8_t *data;
}eb_gap_adv_report_t;

typedef struct {
    uint8_t status;
    uint16_t handle;
    uint16_t interval;
    uint16_t latency;
    uint16_t timeout;
}eb_gap_param_update_t;

typedef struct {
    uint16_t mtu;
}eb_gap_mtu_update_t;

typedef struct {
    union{
        eb_gap_connected_t connected;
        eb_gap_disconnected_t disconnected;
        eb_gap_encrypt_change_t encrypt_change;
        eb_gap_enc_request_t enc_request;
        eb_gap_ltk_request_t ltk_request;
        eb_gap_adv_report_t adv_report;
        eb_gap_param_update_t param_update;
        eb_gap_mtu_update_t mtu_update;
    };
} eb_gap_event_t;


void eb_gap_init(void);
void eb_gap_reset(void);
uint8_t eb_gap_get_address(bdaddr_t addr);
void eb_gap_set_random_address(bdaddr_t addr);
uint8_t eb_gap_get_peer_address(bdaddr_t addr);
uint8_t eb_gap_get_local_address(bdaddr_t addr);
void eb_gap_adv_set_data(eb_adv_set_type_t type, uint8_t *data, uint8_t len);
void eb_gap_adv_set_param(uint16_t intv_min, uint16_t intv_max, eb_adv_type_t adv_type, 
                                    eb_adv_addr_type_t own_addr_type, eb_adv_addr_type_t peer_addr_type,
                                    bdaddr_t *peer_addr, uint8_t eb_adv_chn_map, eb_adv_filter_policy_t filter_policy);
void eb_gap_adv_enable(bool enable);
uint16_t eb_gap_get_conn_handle(void);
// scantype: 0-passive,1-active interval:0.626ms addr_type:0-public,1-random 
void eb_gap_set_scan_param(uint8_t scan_type, uint16_t interval, uint16_t window,
                                        uint8_t addr_type, uint8_t filter_policy);
void eb_gap_scan_enable(bool enable, uint8_t filter_dup);
void eb_gap_connect(bdaddr_t addr, uint8_t addr_type);
void eb_gap_connect_cancel(void);
void eb_gap_disconnect(uint16_t con_hdl, uint8_t reason);
bool eb_gap_get_encrypted(uint16_t con_hdl);
uint16_t eb_gap_get_mtu(void);


#endif /* __EB_GAP_H__ */
