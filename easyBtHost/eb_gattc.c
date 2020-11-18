#include <string.h>
#include "eb_gattc.h"
#include "eb_gap.h"
#include "easyBle.h"

static uint16_t blob_read_offset;
void eb_gattc_init(void)
{
}

void eb_gattc_handler(uint8_t *data, uint16_t len)
{
}

void eb_gattc_error_rsp_handler(uint16_t conn_hd, uint8_t *data, uint16_t len)
{
    eb_event_t evt = { EB_EVT_GATTC_ERROR_RSP };
    evt.gattc.err.conn_hdl = conn_hd;
    evt.gattc.err.att_handle = data[2] + (data[3]<<8);
    evt.gattc.err.req_opcode = data[1];
    evt.gattc.err.err_code = data[4];
    eb_event(&evt);
}

void eb_gattc_read_group_rsp_handler(uint16_t conn_hd, uint8_t *data, uint16_t len)
{
    eb_gattc_service_t serv[3];
    eb_event_t evt = { EB_EVT_GATTC_READ_GROUP_RSP };
    evt.gattc.read_group.conn_hdl = conn_hd;
    evt.gattc.read_group.serv_num = (len-2)/data[1];
    evt.gattc.read_group.serv = serv;
    int i;
    for(i=0;i<evt.gattc.read_group.serv_num;i++){
        serv[i].uuid.is128bit = data[1]!=0x06;
        uint8_t *p = data+2+i*data[1];
        serv[i].att_start_hdl = *p + (*(p+1)<<8);
        p += 2;
        serv[i].att_end_hdl = *p + (*(p+1)<<8);
        p += 2;
        memcpy(&serv[i].uuid.uuid[0], p, data[1]-4);
    }
    eb_event(&evt);
}
void eb_gattc_read_by_type_value_rsp_handler(uint16_t conn_hd, uint8_t *data, uint16_t len)
{
    eb_event_t evt = { EB_EVT_GATTC_FIND_BY_UUID_RSP };
    if(data){
        evt.gattc.find_by_uuid.start_hdl = data[1] + (data[2] << 8);
        evt.gattc.find_by_uuid.end_hdl = data[3] + (data[4] << 8);
    }
    eb_event(&evt);
}
void eb_gattc_read_by_type_rsp_handler(uint16_t conn_hd, uint8_t *data, uint16_t len)
{
    eb_gattc_character_t chars[3];
    eb_event_t evt = { EB_EVT_GATTC_READ_BY_TYPE_RSP };
    evt.gattc.read_by_type.conn_hdl = conn_hd;
    evt.gattc.read_by_type.char_num = (len-2)/data[1];
    evt.gattc.read_by_type.chars= chars;
    int i;
    for(i=0;i<evt.gattc.read_by_type.char_num;i++){
        uint8_t *p = data+2+i*data[1];
        chars[i].att_char_hdl = *p + (*(p+1)<<8);
        p += 2;
        chars[i].properties = *p++;
        chars[i].att_value_hdl = *p + (*(p+1)<<8);
        p += 2;
        chars[i].uuid.is128bit = data[1]!=0x07;
        memcpy(&chars[i].uuid.uuid[0], p, data[1]-5);
    }
    eb_event(&evt);
}
void eb_gattc_find_info_rsp_handler(uint16_t conn_hd, uint8_t *data, uint16_t len)
{
    eb_gattc_info_t infos[5];
    eb_event_t evt = { EB_EVT_GATTC_FIND_INFO_RSP };
    evt.gattc.find_info.conn_hdl = conn_hd;
    evt.gattc.find_info.info_num = (len-2)/(data[1]?4:18);
    evt.gattc.find_info.infos = infos;
    int i;
    for(i=0;i<evt.gattc.find_info.info_num;i++){
        uint8_t *p = data+2+i*(data[1]?4:18);
        infos[i].handle = *p + (*(p+1)<<8);
        p += 2;
        infos[i].uuid.is128bit = !data[1];
        memcpy(&infos[i].uuid.uuid[0], p, data[1]?2:16);
    }
    eb_event(&evt);
}
void eb_gattc_read_rsp_handler(uint16_t conn_hd, uint8_t *data, uint16_t len)
{
    eb_event_t evt = { EB_EVT_GATTC_READ_RSP };
    evt.gattc.read.offset = 0;
    evt.gattc.read.value = data + 1;
    evt.gattc.read.len = len - 1;
    eb_event(&evt);
}

void eb_gattc_read_blob_rsp_handler(uint16_t conn_hd, uint8_t *data, uint16_t len)
{
    eb_event_t evt = { EB_EVT_GATTC_READ_RSP };
    evt.gattc.read.offset = blob_read_offset;
    evt.gattc.read.value = data + 1;
    evt.gattc.read.len = len - 1;
    eb_event(&evt);
}

void eb_gattc_write_rsp_handler(uint16_t conn_hd, uint8_t *data, uint16_t len)
{
}

void eb_gattc_notify_rsp_handler(uint16_t conn_hd, uint8_t *data, uint16_t len)
{
    eb_event_t evt = { EB_EVT_GATTC_NOTIFY };
    evt.gattc.hvx.conn_hdl = conn_hd;
    evt.gattc.hvx.att_hdl = data[1] + (data[2]<<8);
    evt.gattc.hvx.len = len-3;
    evt.gattc.hvx.value = &data[3];
    eb_event(&evt);
}

void eb_gattc_read_group(uint16_t conn_hd, uint16_t att_hd_start, uint16_t att_hd_end)
{
    uint8_t cmd[9+7] = {0x02, conn_hd&0xFF, conn_hd>>8, 0x0b, 0x00, 0x07, 0x00, 0x04, 0x00, 0x10,
        att_hd_start & 0xFF, att_hd_start >> 8, att_hd_end & 0xFF, att_hd_end >> 8, 0x00, 0x28};
    eb_h4_send(cmd, sizeof(cmd));
}

void eb_gattc_read_by_type(uint16_t conn_hd, uint16_t att_hd_start, uint16_t att_hd_end, uuid_t *uuid)
{
    uint8_t cmd[9+21] = {0x02, conn_hd&0xFF, conn_hd>>8,
        uuid->is128bit?0x19:0x0B, 0x00, uuid->is128bit?0x15:0x07, 0x00, 0x04, 0x00, 0x08,
        att_hd_start & 0xFF, att_hd_start >> 8, att_hd_end & 0xFF, att_hd_end >> 8};
    memcpy(&cmd[14], &uuid->uuid[0], uuid->is128bit?16:2);
    eb_h4_send(cmd, uuid->is128bit?30:16);
}

void eb_gattc_find_by_uuid(uint16_t conn_hd, uint16_t att_hd_start, uint16_t att_hd_end, uuid_t *uuid)
{
    uint8_t cmd[9+21] = {0x02, conn_hd&0xFF, conn_hd>>8,
        uuid->is128bit?0x1b:0x0d, 0x00, uuid->is128bit?0x17:0x09, 0x00, 0x04, 0x00, 0x06,
        att_hd_start & 0xFF, att_hd_start >> 8, att_hd_end & 0xFF, att_hd_end >> 8, 0x00, 0x28};
    memcpy(&cmd[16], &uuid->uuid[0], uuid->is128bit?16:2);
    eb_h4_send(cmd, uuid->is128bit?32:18);
}

void eb_gattc_find_info(uint16_t conn_hd, uint16_t att_hd_start, uint16_t att_hd_end)
{
    uint8_t cmd[9+5] = {0x02, conn_hd&0xFF, conn_hd>>8, 0x09, 0x00, 0x05, 0x00, 0x04, 0x00, 0x04,
        att_hd_start & 0xFF, att_hd_start >> 8, att_hd_end & 0xFF, att_hd_end >> 8};
    eb_h4_send(cmd, sizeof(cmd));
}

void eb_gattc_read(uint16_t conn_hd, uint16_t att_hd, uint16_t offset)
{
    if(offset){
        uint8_t cmd[9+5] = {0x02, conn_hd&0xFF, conn_hd>>8, 9, 0x00, 5, 0x00, 0x04, 0x00, 0x0C,
            att_hd & 0xFF, att_hd >> 8, offset & 0xFF, offset >> 8};
        blob_read_offset = offset;
        eb_h4_send(cmd, sizeof(cmd));
    }else{
        uint8_t cmd[9+3] = {0x02, conn_hd&0xFF, conn_hd>>8, 7, 0x00, 3, 0x00, 0x04, 0x00, 0x0A,
            att_hd & 0xFF, att_hd >> 8 };
        eb_h4_send(cmd, sizeof(cmd));
    }
}

void eb_gattc_write(uint16_t conn_hd, uint16_t att_hd, uint8_t *data, uint16_t len)
{
    uint8_t cmd[9+23] = {0x02, conn_hd&0xFF, conn_hd>>8, len+7, 0x00, len+3, 0x00, 0x04, 0x00, 0x12,
        att_hd & 0xFF, att_hd >> 8 };
    len = len<20?len:20;
    memcpy(&cmd[12], data, len);
    eb_h4_send(cmd, len+12);
}

void eb_gattc_write_cmd(uint16_t conn_hd, uint16_t att_hd, uint8_t *data, uint16_t len)
{
    uint8_t cmd[9+23] = {0x02, conn_hd&0xFF, conn_hd>>8, len+7, 0x00, len+3, 0x00, 0x04, 0x00, 0x52,
        att_hd & 0xFF, att_hd >> 8 };
    len = len<20?len:20;
    memcpy(&cmd[12], data, len);
    eb_h4_send(cmd, len+12);
}



