#include <string.h>
#include "eb_gatts.h"
#include "eb_gap.h"
#include "easyBle.h"

void eb_gatts_init(void)
{

}

void eb_gatts_handler(uint8_t *data, uint16_t len)
{

}

void eb_gatts_error_rsp_handler(uint16_t conn_hd, uint8_t *data, uint16_t len)
{
    eb_event_t evt = { EB_EVT_GATTS_ERROR_RSP };
    evt.gattc.err.conn_hdl = conn_hd;
    evt.gattc.err.att_handle = data[2] + (data[3]<<8);
    evt.gattc.err.req_opcode = data[1];
    evt.gattc.err.err_code = data[4];
    eb_event(&evt);
}

void eb_gatts_read_response(uint16_t conn_hd, uint16_t att_hdl, uint16_t offset, uint8_t *data, uint16_t len);
void eb_gatts_read_request_handler(uint16_t conn_hd, uint8_t *data, uint16_t len)
{
    eb_event_t evt = { EB_EVT_GATTS_READ_REQ };
    evt.gatts.read.conn_hdl = conn_hd;
    evt.gatts.read.att_hdl = data[1] + (data[2]<<8);
    evt.gatts.read.offset = 0;
    const eb_att_db_t *att;
    uint16_t att_db_len = eb_att_get_service(&att);
    if(0 < evt.gatts.read.att_hdl && evt.gatts.read.att_hdl <= att_db_len){
        att = &att[evt.gatts.read.att_hdl-1];
        if(!att->prop_read){
            eb_att_error_response(conn_hd, evt.gatts.read.att_hdl, data[0], 0x02); // Read Not Permitted
            return;
        }else if(att->perm_read && !eb_gap_get_encrypted(conn_hd)){
            eb_att_error_response(conn_hd, evt.gatts.read.att_hdl, data[0], 0x05); // Insufficient Authentication
            return;
        }
    }
    evt.gatts.read.value = &data[3]; // WARNING: the pointer of data MUST can be hold all response data.
    eb_event(&evt);
    eb_gatts_read_response(conn_hd, evt.gatts.read.att_hdl, 0, evt.gatts.read.value, evt.gatts.read.length);
}

void eb_gatts_blob_read_request_handler(uint16_t conn_hd, uint8_t *data, uint16_t len)
{
    eb_event_t evt = { EB_EVT_GATTS_READ_REQ };
    evt.gatts.read.conn_hdl = conn_hd;
    evt.gatts.read.att_hdl = data[1] + (data[2]<<8);
    evt.gatts.read.offset = data[3] + (data[4]<<8);
    evt.gatts.read.value = &data[3]; // WARNING: the pointer of data MUST can be hold all response data.
    eb_event(&evt);
    eb_gatts_read_response(conn_hd, evt.gatts.read.att_hdl, evt.gatts.read.offset, evt.gatts.read.value, evt.gatts.read.length);
}

void eb_gatts_read_response(uint16_t conn_hd, uint16_t att_hdl, uint16_t offset, uint8_t *data, uint16_t len)
{
    uint8_t cmd[9+1+len];
    cmd[0] = 0x02;
    cmd[1] = conn_hd&0xFF, cmd[2] = conn_hd>>8;
    cmd[3] = 5 + len;
    cmd[4] = 0;
    cmd[5] = 1 + len;
    cmd[6] = 0;
    cmd[7] = 0x04;
    cmd[8] = 0;
    cmd[9] = offset?0x0D:0x0B;
    memcpy(&cmd[10], data, len);
    eb_h4_send(cmd, 9+1+len);
}

void eb_gatts_write_request_handler(uint16_t conn_hd, uint8_t *data, uint16_t len)
{
    eb_event_t evt;
    evt.gatts.write.conn_hdl = conn_hd;
    evt.gatts.write.att_hdl = data[1] + (data[2]<<8);
    evt.gatts.write.length = len-3;
    evt.gatts.write.value = &data[3];
    const eb_att_db_t *att;
    uint16_t att_db_len = eb_att_get_service(&att);
    if(0 < evt.gatts.write.att_hdl && evt.gatts.write.att_hdl <= att_db_len){
        att = &att[evt.gatts.write.att_hdl-1];
        if(((data[0] & 0x40) && !att->prop_write_cmd) || (!(data[0] & 0x40) && !att->prop_write)){
            eb_att_error_response(conn_hd, evt.gatts.write.att_hdl, data[0], 0x03); // Write Not Permitted
            return;
        }else if(att->perm_write && !eb_gap_get_encrypted(conn_hd)){
            eb_att_error_response(conn_hd, evt.gatts.write.att_hdl, data[0], 0x05); // Insufficient Authentication
            return;
        }
    }
    if(data[0] & 0x40){
        evt.evt_id = EB_EVT_GATTS_WRITE_CMD_REQ;
    }else{
        evt.evt_id = EB_EVT_GATTS_WRITE_REQ;
        uint8_t cmd[9+1] = {0x02, conn_hd&0xFF, conn_hd>>8, 0x05, 0x00, 0x01, 0x00, 0x04, 0x00, 0x13}; // Write Response
        eb_h4_send(cmd, sizeof(cmd));
    }
    eb_event(&evt);
}

void eb_gatts_write_response_handler(uint16_t conn_hd, uint8_t *data, uint16_t len)
{
    eb_event_t evt = { EB_EVT_GATTS_WRITE_RSP };
    eb_event(&evt);
}
void eb_gatts_indicate_rsp_handler(uint16_t conn_hd, uint8_t *data, uint16_t len)
{
    eb_event_t evt = { EB_EVT_GATTS_INDICATE_RSP };
    evt.gatts.read.conn_hdl = conn_hd;
    evt.gatts.read.att_hdl = data[1] + (data[2]<<8);
    eb_event(&evt);
}

static void eb_gatts_send_hvx(uint16_t conn_hd, uint16_t att_hd, uint8_t *data, uint16_t len, uint8_t type)
{
    uint8_t cmd[9+23] = {0x02, conn_hd&0xFF, conn_hd>>8, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00};
    len = len<20?len:20;
    cmd[3] = len + 7;
    cmd[5] = len + 3;
    cmd[9] = type;
    cmd[10] = att_hd & 0xFF;
    cmd[11] = att_hd >> 8;
    memcpy(&cmd[12], data, len);
    eb_h4_send(cmd, cmd[3]+5);
}

void eb_gatts_send_notify(uint16_t conn_hd, uint16_t att_hd, uint8_t *data, uint16_t len)
{
    eb_gatts_send_hvx(conn_hd, att_hd, data, len, 0x1B);
}

void eb_gatts_send_indicate(uint16_t conn_hd, uint16_t att_hd, uint8_t *data, uint16_t len)
{
    eb_gatts_send_hvx(conn_hd, att_hd, data, len, 0x1D);
}



