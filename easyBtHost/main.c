#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "easyBle.h"

#define APP_MASTER 0

uint16_t pending_serv_st_hdl;
uint16_t pending_serv_en_hdl;

uint16_t conn_handle;
static bool conn_flag;
bool timeout(uint8_t id, void*p)
{
    static int cnt = 0;
    if(l2cap_packet_num()<4){
        printf("notify cnt: %d\n", ++cnt);
        eb_gatts_send_notify(conn_handle , 5, (uint8_t*)"666666", 6);
        //eb_gatts_send_indicate(conn_handle, 5, "77777", 5);
    }
    if(id){
        return true;
    }else{
        eb_set_timer(1, 50, timeout, NULL);
        return false;
    }
}
void dump_uuid(uuid_t *uuid, char *surfix)
{
    if(uuid->is128bit){
        int i; printf("0x"); for(i=0;i<16;i++){
            printf("%02X ", uuid->uuid[16-i-1]);
        }
    }else{
        printf("0x%02X", uuid->uuid[0]+(uuid->uuid[1]<<8));
    }
    printf("%s", surfix);
}
void ble_event_cb(eb_event_t *param)
{
    //printf("Evt_ID:0x%04X\n", param->evt_id);
#if APP_MASTER == 1
    eb_set_timer(1, 10000, timeout, NULL);
#endif
    switch(param->evt_id){
        case EB_EVT_GAP_RESET:{
#if APP_MASTER == 0
            eb_gap_adv_set_data(EB_GAP_ADV_SET_DATA, (uint8_t*)"\x02\x01\x06\x03\x03\x12\x18\x03\x19\xC2\x03", 11);
            usleep(5000);
            eb_gap_adv_set_data(EB_GAP_ADV_SET_SCAN_RSP, (uint8_t*)"\x02\x09\x5F", 3);
            usleep(5000);
            eb_gap_adv_set_param(0x30, 0x40, EB_GAP_ADV_IND, EB_ADV_ADDR_TYPE_RANDOM, 0, NULL, 0x07, EB_ADV_FILTER_DEFAULT);
            usleep(5000);
            bdaddr_t bdaddr = {0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6};
            eb_gap_set_random_address(bdaddr);
            usleep(5000);
            eb_gap_adv_enable(true);
#else
            conn_flag = false;
            bdaddr_t bdaddr = {(rand()&0x7F)|0x40, rand()%0xFF, rand()%0xFF, rand()%0xFF, rand()%0xFF, rand()%0xFF};
            eb_gap_set_random_address(bdaddr);
            usleep(5000);
            eb_gap_set_scan_param(0, 0x10, 0x10, 1, 0);
            eb_gap_scan_enable(true, 1);
#endif
            break;}
        case EB_EVT_GAP_CONNECTED:{
            printf("Connected.\n");
            conn_handle = param->gap.connected.handle;
            eb_gattc_read_group(conn_handle, 0x0001, 0xffff);
            //eb_set_timer(0, 5000, timeout, NULL);
            break;}
        case EB_EVT_GAP_DISCONNECTED:
            eb_del_timer(0);
            eb_gap_reset();
            break;
        case EB_EVT_GAP_ENC_REQUEST:{
            int i;
            for(i=0;i<10;i++){ param->gap.enc_request.ediv[i] = rand()&0xFF; }
            memcpy(&param->gap.enc_request.ltk[0], param->gap.enc_request.random, 8);
            memcpy(&param->gap.enc_request.ltk[8], param->gap.enc_request.random, 8);
            break;}
        case EB_EVT_GAP_LTK_REQUEST:{
            param->gap.ltk_request.ltk_found = true;
            memcpy(&param->gap.ltk_request.ltk[0], param->gap.ltk_request.random, 8);
            memcpy(&param->gap.ltk_request.ltk[8], param->gap.ltk_request.random, 8);
            break;}
        case EB_EVT_GATTS_READ_REQ:
            memcpy(param->gatts.read.value, "666", 3);
            param->gatts.read.length = 3;
            break;
        case EB_EVT_GATTS_WRITE_REQ:
            //eb_gatts_send_notify(param->gatts.write.conn_hdl, param->gatts.write.att_hdl,
            //                    (uint8_t*)"1111111111", 10);
            break;
        case EB_EVT_GATTS_WRITE_CMD_REQ:
            eb_gatts_send_indicate(param->gatts.write.conn_hdl, param->gatts.write.att_hdl,
                                (uint8_t*)"6666666666", 10);
        case EB_EVT_GAP_ADV_REPORT:
            if(!conn_flag && param->gap.adv_report.type == EB_GAP_REPORT_IND
                /* && param->gap.adv_report.rssi < -80*/){
                printf("Type: %d, ", param->gap.adv_report.addr_type);
                for(int i=5;i>0;i--) printf("%02X:", param->gap.adv_report.addr[i]);
                printf("%02X %d\n", param->gap.adv_report.addr[0], param->gap.adv_report.rssi);
                eb_gap_scan_enable(false, 1);
                usleep(50000);
                uint8_t cmd[] = {
                    0x01, 0x0D, 0x20, 0x19, 0x30, 0x00, 0x30, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x01,
                    0x10, 0x00, 0x20, 0x00, 0,0, 0xB8,0x0B, 0,0, 0,0,
                };
                cmd[9] = param->gap.adv_report.addr_type;
                memcpy(&cmd[10], &param->gap.adv_report.addr, 6);
                eb_h4_send(cmd, sizeof(cmd));
                eb_set_timer(0, 2000, timeout, NULL);
                conn_flag = true;
            }
            break;
        case EB_EVT_GATTC_READ_GROUP_RSP:{
            eb_gattc_service_t *p = param->gattc.read_group.serv;
            printf("-------------------------\nS | 0x%04X ", p->att_start_hdl);
            dump_uuid(&p->uuid, "");
            printf("  (0x%04X-0x%04X)\n", p->att_start_hdl, p->att_end_hdl);
            uuid_t uuid = {0, {0x03, 0x28}};
            eb_gattc_read_by_type(conn_handle, p->att_start_hdl, p->att_end_hdl, &uuid);
            pending_serv_st_hdl = p->att_start_hdl;
            pending_serv_en_hdl = p->att_end_hdl;
            break;}
        case EB_EVT_GATTC_READ_BY_TYPE_RSP:{
            if(pending_serv_st_hdl >= pending_serv_en_hdl){ break; }
            eb_gattc_character_t *p = param->gattc.read_by_type.chars;
            printf("C | 0x%04X ", p->att_char_hdl);
            dump_uuid(&p->uuid, "");
            printf(" 0x%02X\n", p->properties);
            printf("C | 0x%04X \"\"\n", p->att_value_hdl);
            if(p->att_value_hdl == pending_serv_en_hdl && pending_serv_en_hdl != 0xFFFF){
                eb_gattc_read_group(conn_handle, pending_serv_en_hdl+1, 0xffff);
            }else{
                eb_gattc_find_info(conn_handle, p->att_value_hdl+1, pending_serv_en_hdl);
            }
            break;}
        case EB_EVT_GATTC_FIND_INFO_RSP:{
            if(pending_serv_st_hdl >= pending_serv_en_hdl){ break; }
            eb_gattc_info_t *p = param->gattc.find_info.infos;
            if(!p->uuid.is128bit){
                uint16_t uuid = p->uuid.uuid[0]+(p->uuid.uuid[1]<<8);
                if(0x2900<=uuid&&uuid<=0x2905){
                    printf("D | 0x%04X ", p->handle);
                    dump_uuid(&p->uuid, "\n");
                    if(p->handle < pending_serv_en_hdl){
                        if(p->handle != 0xFFFF){
                            eb_gattc_find_info(conn_handle, p->handle+1, pending_serv_en_hdl);
                        }
                        break;
                    }
                }
            }
            if(p->handle < pending_serv_en_hdl){
                uuid_t uuid = {0, {0x03, 0x28}};
                eb_gattc_read_by_type(conn_handle, p->handle, pending_serv_en_hdl, &uuid);
            }else{
                eb_gattc_read_group(conn_handle, pending_serv_en_hdl, 0xffff);
            }
            break;}
        case EB_EVT_GATTC_ERROR_RSP:{
            eb_gattc_err_t *p = &param->gattc.err;
            if(p->err_code == 0x0A && pending_serv_st_hdl < pending_serv_en_hdl){ // ATT not found
                switch(p->req_opcode){
                    case 0x10:{ // Read by group type request
                        pending_serv_st_hdl = 0;
                        pending_serv_en_hdl = 0;
                        break;}
                    case 0x08:{ // Find By Type request
                        eb_gattc_read_group(conn_handle, pending_serv_en_hdl+1, 0xffff);
                        break;}
                    case 0x04:{ // Find Infomation request
                        uuid_t uuid = {0, {0x03, 0x28}};
                        eb_gattc_read_by_type(conn_handle, p->att_handle, pending_serv_en_hdl, &uuid);
                        break;}
                }
            }
            break;}
    }
}

const uuid16_t serv1  = {0x66, 0x33};
const uuid16_t uuid1  = {0x00, 0x2A};
const uuid128_t serv2 = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
const uuid128_t uuid2 = {0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02};
uint8_t uuid2_value[20];
uint8_t desc2_value[2];


const eb_att_db_t att_db[] = {
    {{&ATT_DECL_PRIMARY_SERVICE}, (void*)&serv1,       1,0,  1,0,0,0,0, 0,0,},
    {{&ATT_DECL_CHARACTERISTIC},  NULL,                0,0,  1,0,0,0,0, 0,0,},
    {{&uuid1},                    NULL,                0,0,  1,1,1,1,1, 0,0,},
    {{&ATT_DESC_CLIENT_CHAR_CFG}, NULL,                0,0,  1,1,0,0,0, 0,0,},
    {{&ATT_DECL_PRIMARY_SERVICE}, (void*)&serv2,       1,1,  1,1,1,1,1, 0,0,},
    {{&ATT_DECL_CHARACTERISTIC},  NULL,                0,0,  1,0,0,0,0, 0,0,},
    {{(const uuid16_t*)&uuid2},   NULL,                0,1,  1,0,1,1,1, 0,0,},
    {{&ATT_DESC_CLIENT_CHAR_CFG}, (void*)&desc2_value, 0,0,  1,1,0,0,0, 0,1,},
};

static void handle_sigint(int sig)
{
    eb_gap_reset();
    usleep(5000);
    exit(0);
}

int main(void)
{
    signal(SIGINT, handle_sigint);
    eb_init(ble_event_cb);
    eb_att_set_service(att_db, sizeof(att_db)/sizeof(att_db[0]));
    while(1){ eb_schedule(); }
}
