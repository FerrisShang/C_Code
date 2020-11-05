#include <string.h>
#include "easyBle.h"
#include "eb_att.h"
#include "eb_gatts.h"

const uuid16_t ATT_DECL_PRIMARY_SERVICE        = {0x00, 0x28};
const uuid16_t ATT_DECL_CHARACTERISTIC         = {0x03, 0x28};
const uuid16_t ATT_DESC_CHAR_USER_DESCRIPTION  = {0x01, 0x29};
const uuid16_t ATT_DESC_CLIENT_CHAR_CFG        = {0x02, 0x29};
const uuid16_t ATT_DESC_REPORT_REF             = {0x08, 0x29};

static eb_att_db_t const *eb_att_db;
static uint16_t eb_att_db_len;
extern void eb_gatts_indicate_rsp_handler(uint16_t conn_hd, uint8_t *data, uint16_t len);


void eb_att_init(void)
{
}
void eb_att_error_response(uint16_t conn_hd, uint16_t att_hd, uint8_t opcode, uint8_t err_code)
{
    uint8_t cmd[9+5] = {0x02, conn_hd&0xFF, conn_hd>>8, 0x09, 0x00, 0x05, 0x00, 0x04, 0x00,
                        0x01, opcode, att_hd&0xFF, att_hd>>8, err_code};
    eb_h4_send(cmd, sizeof(cmd));
}
static void eb_att_mtu_request_handler(uint16_t conn_hd, uint16_t mtu)
{
    uint16_t mtu_support = 23;
    uint8_t cmd[9+3] = {0x02, conn_hd&0xFF, conn_hd>>8, 0x07, 0x00, 0x03, 0x00, 0x04, 0x00,
                        0x03, mtu_support & 0xFF, mtu_support >> 8};
    eb_h4_send(cmd, sizeof(cmd));
}
static void eb_att_read_by_group_request_handler(uint16_t conn_hd, uint16_t sh, uint16_t eh)
{
    uint8_t cmd[9+23] = {0x02, conn_hd&0xFF, conn_hd>>8, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x11};
    uint8_t isUuid128=0xFF, offset = 2; // opcode & len
    uint16_t i, cur_serv;
    if(!eb_att_db_len){ // No service
        eb_att_error_response(conn_hd, sh, 0x10, 0x0A); // ATT not found
        return;
    }
    for(i=sh-1, cur_serv=HANDLE_INVALID;offset<EB_ATT_MTU_DEFAULT && i<=eb_att_db_len && i<=eh;i++){
        if(i == eh || i == eb_att_db_len  || eb_att_db[i].is_service){ // Only primary service support
            if(cur_serv != HANDLE_INVALID){
                cmd[offset+9] = (cur_serv+1)&0xFF;
                cmd[offset+10] = (cur_serv+1)>>8;
                cmd[offset+11] = (i)&0xFF;
                cmd[offset+12] = (i)>>8;
                memcpy(&cmd[offset+13], eb_att_db[cur_serv].value, eb_att_db[cur_serv].uuid_len?16:2);
                offset += eb_att_db[cur_serv].uuid_len?20:6;
            }
            cur_serv = i;
            if(i == eh || i == eb_att_db_len){ break; }
            if(isUuid128==0xFF){
                isUuid128 = eb_att_db[i].uuid_len;
            }else{
                if(isUuid128 != eb_att_db[i].uuid_len){ break; }
            }
        }
    }
    if(offset == 2){ // No service found
        eb_att_error_response(conn_hd, sh, 0x10, 0x0A); // ATT not found
    }else{
        cmd[3] = offset + 4;
        cmd[5] = offset;
        cmd[10] = isUuid128?20:6;
        eb_h4_send(cmd, cmd[3]+5);
    }
}
static void eb_att_read_by_type_request_char_handler(uint16_t conn_hd, uint16_t sh, uint16_t eh)
{
    // Only 0x2803, GATT Characteristic Declaration support !
    uint8_t cmd[9+23] = {0x02, conn_hd&0xFF, conn_hd>>8, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x09};
    uint8_t isUuid128=0xFF, offset = 2; // opcode & len
    uint16_t i;
    for(i=sh-1;i<eb_att_db_len && i<eh;i++){
        if(!eb_att_db[i].uuid_len && *eb_att_db[i].uuid16 == ATT_DECL_CHARACTERISTIC){
            if(offset + (eb_att_db[i+1].uuid_len?21:7) > EB_ATT_MTU_DEFAULT){
                break;
            }
            cmd[offset+9] = (i+1)&0xFF;
            cmd[offset+10] = (i+1)>>8;
            uint8_t prop = (eb_att_db[i+1].prop_read      << 1) +
                           (eb_att_db[i+1].prop_write_cmd << 2) +
                           (eb_att_db[i+1].prop_write     << 3) +
                           (eb_att_db[i+1].prop_ntf       << 4) +
                           (eb_att_db[i+1].prop_ind       << 5);
            cmd[offset+11] = prop;
            cmd[offset+12] = (i+2)&0xFF;
            cmd[offset+13] = (i+2)>>8;
            memcpy(&cmd[offset+14], eb_att_db[i+1].uuid16, eb_att_db[i+1].uuid_len?16:2);
            offset += eb_att_db[i+1].uuid_len?21:7;
            if(i+1 >= eh || i+1 >= eb_att_db_len){ break; }
            if(isUuid128==0xFF){
                isUuid128 = eb_att_db[i+1].uuid_len;
            }else{
                if(isUuid128 != eb_att_db[i+1].uuid_len){ break; }
            }
            i++;
        }
    }
    if(offset == 2){ // No service found
        eb_att_error_response(conn_hd, sh, 0x08, 0x0A); // ATT not found
    }else{
        cmd[3] = offset + 4;
        cmd[5] = offset;
        cmd[10] = isUuid128?21:7;
        eb_h4_send(cmd, cmd[3]+5);
    }
}
static void eb_att_find_by_type_value_request_handler(uint16_t conn_hd, uint16_t sh, uint16_t eh,
        uint16_t type, uint8_t *uuid, uint8_t isUuid128)
{
    if(type != 0x2800){ // TODO: Only support service find now
        eb_att_error_response(conn_hd, sh, 0x06, 0x06);
        return;
    }
    uint16_t st_hdl=0xFFFF, en_hdl;
    for(en_hdl=sh-1;en_hdl<eb_att_db_len && en_hdl<eh;en_hdl++){
        int i = en_hdl;
        if(eb_att_db[i].is_service){
            if(st_hdl!=0xFFFF){ break; }
            uint8_t len = (eb_att_db[i].uuid_len & isUuid128)?16:2;
            if(!memcmp(eb_att_db[i].value, uuid, len)){
                st_hdl = i+1;
            }
        }
    }
    if(st_hdl != 0xFFFF){
        uint8_t cmd[9+5] = {0x02, conn_hd&0xFF, conn_hd>>8, 0x09, 0x00, 0x05, 0x00, 0x04, 0x00, 0x07,
                                st_hdl&0xFF, st_hdl>>8, en_hdl&0xFF, en_hdl>>8};
        eb_h4_send(cmd, cmd[3]+5);
    }else{
        eb_att_error_response(conn_hd, sh, 0x06, 0x0A); // ATT not found
    }
}
static void eb_att_read_by_type_request_value_handler(uint16_t conn_hd, uint16_t sh, uint16_t eh, uint8_t *uuid, uint8_t isUuid128)
{
    uint8_t cmd[9+23] = {0x02, conn_hd&0xFF, conn_hd>>8, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x09};
    uint16_t i;
    for(i=sh-1;i<eb_att_db_len && i<eh;i++){
        if(!memcmp(eb_att_db[i].uuid16, uuid, isUuid128?16:2)){
            eb_event_t evt = { EB_EVT_GATTS_READ_REQ };
            evt.gatts.read.conn_hdl = conn_hd;
            evt.gatts.read.att_hdl = i+1;
            evt.gatts.read.offset = 0;
            evt.gatts.read.value = &cmd[10];
            eb_event(&evt);
            if(evt.gatts.read.length>0){
                uint8_t len = evt.gatts.read.length < 19?evt.gatts.read.length:19;
                cmd[3] = len + 8;
                cmd[5] = len + 4;
                cmd[10] = len + 2;
                cmd[11] = (i+1)&0xFF;
                cmd[12] = (i+1)>>8;
                memcpy(&cmd[10], evt.gatts.read.value, len);
                eb_h4_send(cmd, cmd[3]+5);
            }else{
                eb_att_error_response(conn_hd, sh, 0x08, 0x0A); // ATT not found
            }
            return;
        }
    }
    eb_att_error_response(conn_hd, sh, 0x08, 0x0A); // ATT not found
}
static void eb_att_find_info_request_handler(uint16_t conn_hd, uint16_t sh, uint16_t eh)
{
    uint8_t cmd[9+23] = {0x02, conn_hd&0xFF, conn_hd>>8, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x05};
    uint8_t isUuid128=0xFF, offset = 2; // opcode & len
    uint16_t i;
    for(i=sh-1;i<eb_att_db_len && i<eh;i++){
        if((eb_att_db[i].uuid_len?18:4) > EB_ATT_MTU_DEFAULT){
            break;
        }
        cmd[offset+9] = (i+1)&0xFF;
        cmd[offset+10] = (i+1)>>8;
        memcpy(&cmd[offset+11], eb_att_db[i].uuid16, eb_att_db[i].uuid_len?16:2);
        offset += eb_att_db[i].uuid_len?18:4;
        if(i >= eh || i >= eb_att_db_len){ break; }
        if(isUuid128==0xFF){
            isUuid128 = eb_att_db[i].uuid_len;
        }else{
            if(isUuid128 != eb_att_db[i].uuid_len){ break; }
        }
    }
    if(offset == 2){ // No attribute found
        eb_att_error_response(conn_hd, sh, 0x04, 0x0A); // ATT not found
    }else{
        cmd[3] = offset + 4;
        cmd[5] = offset;
        cmd[10] = isUuid128?2:1;
        eb_h4_send(cmd, cmd[3]+5);
    }
}

void eb_att_handler(uint8_t *data, uint16_t len)
{
    uint16_t conn_hd = (data[1] + (data[2]<<8)) & 0x0FFF;
    uint8_t opcode = data[9];
    uint16_t sh = data[10] + (data[11]<<8);
    uint16_t eh = data[12] + (data[13]<<8);
    switch(opcode){
        case 0x01:{ // Error response
            if((data[10]&0x3F) >= 0x1B){
                eb_gatts_error_rsp_handler(conn_hd, &data[9], len-9);
            }else{
                eb_gattc_error_rsp_handler(conn_hd, &data[9], len-9);
            }
            break;}
        case 0x02:{ // Exchange MTU request
            uint16_t mtu = data[10] + (data[11]<<8);
            eb_att_mtu_request_handler(conn_hd, mtu);
            break;}
        case 0x04:{ // Find Infomation request
            eb_att_find_info_request_handler(conn_hd, sh, eh);
            break;}
        case 0x05:{ // Find Infomation response
            eb_gattc_find_info_rsp_handler(conn_hd, &data[9], len-9);
            break;}
        case 0x06:{ // Find By Type Value request
            uint16_t att_type = data[14] + (data[15]<<8);
            eb_att_find_by_type_value_request_handler(conn_hd, sh, eh, att_type, &data[16], len>18?1:0);
            break;}
        case 0x07:{ // Find By Type Value response
            eb_gattc_read_by_type_value_rsp_handler(conn_hd, &data[9], len-9);
            break;}
        case 0x08:{ // Read by type request
            uint16_t uuid = data[14] + (data[15]<<8);
            if(uuid != 0x2803){
                eb_att_read_by_type_request_value_handler(conn_hd, sh, eh, &data[14], len>16?1:0);
            }else{
                eb_att_read_by_type_request_char_handler(conn_hd, sh, eh);
            }
            break;}
        case 0x09:{ // Read by type response
            eb_gattc_read_by_type_rsp_handler(conn_hd, &data[9], len-9);
            break;}
        case 0x10:{ // Read by group type request
            uint16_t uuid = data[14] + (data[15]<<8);
            if(uuid == 0x2800){ // Primary service
                eb_att_read_by_group_request_handler(conn_hd, sh, eh);
            }else{
                eb_att_error_response(conn_hd, sh, opcode, 0x0A); // ATT not found
            }
            break;}
        case 0x11:{ // Read by group type response
            eb_gattc_read_group_rsp_handler(conn_hd, &data[9], len-9);
            break;}
        case 0x0A:{ // Read Request
            eb_gatts_read_request_handler(conn_hd, &data[9], len-9);
            break;}
        case 0x0B:{ // Read Response
            eb_gattc_read_rsp_handler(conn_hd, &data[9], len-9);
            break;}
        case 0x0C:{ // Read Blob Request
            eb_gatts_blob_read_request_handler(conn_hd, &data[9], len-9);
            break;}
        case 0x0D:{ // Read Blob Response
            eb_gattc_read_blob_rsp_handler(conn_hd, &data[9], len-9);
            break;}
        case 0x12:
        case 0x52:{ // Write Request
            eb_gatts_write_request_handler(conn_hd, &data[9], len-9);
            break;}
        case 0x13:{ // Write Response
            eb_gatts_write_response_handler(conn_hd, &data[9], len-9);
            break;}
        case 0x1B:{ // Notify
            eb_gattc_notify_rsp_handler(conn_hd, &data[9], len-9);
            break;}
        case 0x1D:{ // Indicataion
            eb_gatts_indicate_rsp_handler(conn_hd, &data[9], len-9);
            break;}
        case 0x1E:{ // Indicataion Confirm
            // Maybe should generate a event to upper
            break;}
        default:
            eb_att_error_response(conn_hd, 0x0001, opcode, 0x06); // Request Not Supported
    }
}

void eb_att_set_service(const eb_att_db_t* db, uint16_t len)
{
    eb_att_db = db;
    eb_att_db_len = len;
}

uint16_t eb_att_get_service(const eb_att_db_t **db)
{
    *db = eb_att_db;
    return eb_att_db_len;
}

