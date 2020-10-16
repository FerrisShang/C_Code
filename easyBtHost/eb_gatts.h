#ifndef __EB_GATTS_H__
#define __EB_GATTS_H__

#include "eb_config.h"

enum{
    EB_EVT_GATTS_ERROR_RSP = (EB_GATTS_ID << 8),
    EB_EVT_GATTS_READ_REQ,
    EB_EVT_GATTS_READ_RSP,
    EB_EVT_GATTS_READ_BLOB_REQ,
    EB_EVT_GATTS_READ_BLOB_RSP,
    EB_EVT_GATTS_WRITE_REQ,
    EB_EVT_GATTS_WRITE_CMD_REQ,
    EB_EVT_GATTS_WRITE_RSP,
    EB_EVT_GATTS_INDICATE_RSP,
};

typedef struct {
    uint16_t handle;
    uint8_t req_opcode;
    uint8_t err_code;
}eb_gatts_err_t;

typedef struct {
    uint16_t conn_hdl;
    uint16_t att_hdl;
    uint16_t offset;
    uint16_t length;
    uint8_t *value;
}eb_gatts_read_req_t;

typedef struct {
    uint16_t conn_hdl;
    uint16_t att_hdl;
    uint16_t length;
    uint8_t *value;
}eb_gatts_write_req_t;

typedef struct {
    uint16_t conn_hdl;
    uint16_t att_hdl;
    uint16_t length;
    uint8_t *value;
}eb_gatts_indicate_t;

typedef struct {
    uint16_t conn_hdl;
    uint16_t att_hdl;
}eb_gatts_indicate_rsp_t;


typedef struct {
    union{
        eb_gatts_err_t err;
        eb_gatts_read_req_t read;
        eb_gatts_write_req_t write;
        eb_gatts_write_req_t write_cmd;
        eb_gatts_indicate_rsp_t ind_rsp;
    };
} eb_gatts_event_t;

void eb_gatts_init(void);
void eb_gatts_handler(uint8_t *data, uint16_t len);

void eb_gatts_error_rsp_handler(uint16_t conn_hd, uint8_t *data, uint16_t len);
void eb_gatts_read_request_handler(uint16_t conn_hd, uint8_t *data, uint16_t len);
void eb_gatts_blob_read_request_handler(uint16_t conn_hd, uint8_t *data, uint16_t len);
void eb_gatts_write_request_handler(uint16_t conn_hd, uint8_t *data, uint16_t len);
void eb_gatts_write_response_handler(uint16_t conn_hd, uint8_t *data, uint16_t len);
void eb_gatts_indicate_handler(uint16_t conn_hd, uint8_t *data, uint16_t len);
void eb_gatts_send_notify(uint16_t conn_hd, uint16_t att_hd, uint8_t *data, uint16_t len);
void eb_gatts_send_indicate(uint16_t conn_hd, uint16_t att_hd, uint8_t *data, uint16_t len);

#endif /* __EB_GATTS_H__ */
