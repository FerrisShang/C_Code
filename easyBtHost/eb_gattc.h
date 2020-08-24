#ifndef __EB_GATTC_H__
#define __EB_GATTC_H__

#include "eb_config.h"

enum{
    EB_EVT_GATTC_ERROR_RSP = (EB_GATTC_ID << 8),
    EB_EVT_GATTC_READ_GROUP_RSP,
    EB_EVT_GATTC_FIND_INFO_RSP,
    EB_EVT_GATTC_READ_BY_TYPE_VALUE_RSP,
    EB_EVT_GATTC_READ_BY_TYPE_RSP,
    EB_EVT_GATTC_READ_RSP,
    EB_EVT_GATTC_WRITE_RSP,
    EB_EVT_GATTC_CONFIRM,
};

typedef struct {
    uint16_t is128bit;
    uint8_t uuid[16];
} uuid_t;

typedef struct {
    uint16_t conn_hdl;
    uint16_t att_handle;
    uint8_t req_opcode;
    uint8_t err_code;
}eb_gattc_err_t;

typedef struct {
    uint16_t att_start_hdl;
    uint16_t att_end_hdl;
    uuid_t uuid;

}eb_gattc_service_t;

typedef struct {
    uint16_t conn_hdl;
    uint8_t serv_num;
    eb_gattc_service_t *serv;
}eb_gattc_read_group_rsp_t;

typedef struct {
    uint16_t handle;
    uuid_t uuid;
}eb_gattc_info_t;

typedef struct {
    uint16_t conn_hdl;
    uint8_t info_num;
    eb_gattc_info_t *infos;
}eb_gattc_find_info_rsp_t;

typedef struct {
    uint16_t conn_hdl;
}eb_gattc_read_by_type_value_rsp_t;

typedef struct {
    uint16_t att_char_hdl;
    uint8_t properties;
    uint16_t att_value_hdl;
    uuid_t uuid;
}eb_gattc_character_t;

typedef struct {
    uint16_t conn_hdl;
    uint8_t char_num;
    eb_gattc_character_t *chars;
}eb_gattc_read_by_type_rsp_t;

typedef struct {
    uint16_t conn_hdl;
}eb_gattc_read_rsp_t;

typedef struct {
    uint16_t conn_hdl;
}eb_gattc_write_rsp_t;

typedef struct {
    uint16_t conn_hdl;
}eb_gattc_confirm_t;


typedef struct {
    union{
        eb_gattc_err_t err;
        eb_gattc_read_group_rsp_t read_group;
        eb_gattc_find_info_rsp_t find_info;
        eb_gattc_read_by_type_value_rsp_t read_by_type_value;
        eb_gattc_read_by_type_rsp_t read_by_type;
        eb_gattc_read_rsp_t read;
        eb_gattc_write_rsp_t write;
        eb_gattc_confirm_t confirm;
    };
} eb_gattc_event_t;


void eb_gattc_init(void);
void eb_gattc_handler(uint8_t *data, uint16_t len);

void eb_gattc_error_rsp_handler(uint16_t conn_hd, uint8_t *data, uint16_t len);
void eb_gattc_read_group_rsp_handler(uint16_t conn_hd, uint8_t *data, uint16_t len);
void eb_gattc_read_by_type_value_rsp_handler(uint16_t conn_hd, uint8_t *data, uint16_t len);
void eb_gattc_read_by_type_rsp_handler(uint16_t conn_hd, uint8_t *data, uint16_t len);
void eb_gattc_find_info_rsp_handler(uint16_t conn_hd, uint8_t *data, uint16_t len);
void eb_gattc_read_rsp_handler(uint16_t conn_hd, uint8_t *data, uint16_t len);
void eb_gattc_write_rsp_handler(uint16_t conn_hd, uint8_t *data, uint16_t len);

void eb_gattc_read_group(uint16_t conn_hd, uint16_t att_hd_start, uint16_t att_hd_end);
void eb_gattc_read_by_type(uint16_t conn_hd, uint16_t att_hd_start, uint16_t att_hd_end, uuid_t *uuid);
void eb_gattc_find_info(uint16_t conn_hd, uint16_t att_hd_start, uint16_t att_hd_end);
void eb_gattc_read(uint16_t conn_hd, uint16_t att_hd, uint16_t offset);
void eb_gattc_write(uint16_t conn_hd, uint16_t att_hd, uint8_t *data, uint16_t len);
void eb_gattc_write_cmd(uint16_t conn_hd, uint16_t att_hd, uint8_t *data, uint16_t len);
void eb_gattc_confirm(uint16_t conn_hd, uint16_t att_hd);


#endif /* __EB_GATTC_H__ */
