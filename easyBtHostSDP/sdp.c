#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <assert.h>
#include "uuid_str.h"
#include "easyBle.h"

static uint16_t conn_handle;
#define MAX_VAL_LEN 0x1000
#define MAX_ATT_NUM 0x10000

enum {
    AT_NONE,
    AT_SERV,
    AT_CHARD,
    AT_CHARV,
    AT_DESC,
};
typedef struct {
    uint16_t handle;
    uint16_t serv_end_hdl;
    uint8_t att_type;
    uint8_t uuid[16];
    uint8_t uuid_len;
    uint8_t value[MAX_VAL_LEN];
    uint8_t value_len;
    uint8_t properties;
    uint8_t perm_read;
    uint8_t perm_write;
} gatt_db_t;
static gatt_db_t db[MAX_ATT_NUM];
static int p_hdl1, p_hdl2, p_hdl3;

void print_uuid(uint8_t* uuid, int len)
{
    if (len > 2) {
        int i; printf("0x");
        for (i = 0; i < 16; i++) {
            printf("%02X", uuid[16 - i - 1]);
        }
    } else {
        printf("0x%04X", uuid[0] + (uuid[1] << 8));
    }
}

void dump_db(void)
{
    //if(!db){ return; }
    int i;
    for (i = 1; i < MAX_ATT_NUM; i++) {
        switch (db[i].att_type) {
            case AT_SERV:
                printf("-------------\n");
                printf("S | 0x%04X ", db[i].handle);
                print_uuid(db[i].uuid, db[i].uuid_len);
                printf("  :0x%04X~0x%04X  ", db[i].handle, db[i].serv_end_hdl);
                print_uuid_str(db[i].uuid, db[i].uuid_len);
                printf("\n");
                break;
            case AT_CHARD:
                printf("C | 0x%04X ", db[i].handle);
                print_uuid(db[i].uuid, db[i].uuid_len);
                printf("  ");
                if (db[i].perm_read) {
                    printf("R!");
                } else {
                    printf("  ");
                }
                if (db[i].perm_write) {
                    printf("W!  ");
                } else {
                    printf("    ");
                }
                printf("prop(0x%02X):     ", db[i].properties);
                print_uuid_str(db[i+1].uuid, db[i+1].uuid_len);
                puts("");
                break;
            case AT_CHARV:
                printf("C | 0x%04X ", db[i].handle);
                print_uuid(db[i].uuid, db[i].uuid_len);
                printf("        Value: ");
                if (db[i].value_len) {
                    uint8_t isStr = 1;
                    for (int j = 0; j < db[i].value_len; j++) {
                        if (j > 0 && (!(j % 16))) {
                            printf("\n                                ");
                        }
                        printf("%02X ", db[i].value[j]);
                        if(!isprint(db[i].value[j])){isStr = 0; }
                    }
                    if(isStr){ printf(": '%s'", db[i].value); }
                } else {
                    printf("(NULL)");
                }
                printf("\n");
                break;
            case AT_DESC:
                printf("D | 0x%04X ", db[i].handle);
                print_uuid(db[i].uuid, db[i].uuid_len);
                printf("  ");
                if (db[i].perm_read) {
                    printf("R!");
                } else {
                    printf("  ");
                }
                if (db[i].perm_write) {
                    printf("W!  ");
                } else {
                    printf("    ");
                }
                printf("Value: ");
                if (db[i].value_len) {
                    uint8_t isStr = 1;
                    for (int j = 0; j < db[i].value_len; j++) {
                        if (j > 0 && (!(j % 16))) {
                            printf("\n                                ");
                        }
                        printf("%02X ", db[i].value[j]);
                        if(!isprint(db[i].value[j])){isStr = 0; }
                    }
                    if(isStr){ printf(": '%s'", db[i].value); }
                } else {
                    printf("(NULL)");
                }
                printf("\n");
                break;
        }
    }
    //free(db);
    //db = NULL;
}

void db_write(int st_hdl)
{
    for (; st_hdl <= p_hdl2; st_hdl++) {
        if ((db[st_hdl].att_type == AT_CHARV && (db[st_hdl].properties & 0x08)) ||
                db[st_hdl].att_type == AT_DESC) {
            break;
        }
    }
    if (st_hdl <= p_hdl2) {
        p_hdl1 = st_hdl;
        eb_gattc_write(conn_handle, st_hdl, db[st_hdl].value, db[st_hdl].value_len);
    } else {
        eb_smp_auth(conn_handle);
    }
}
void db_read(int st_hdl, int offset)
{
    if (eb_gap_get_encrypted(conn_handle)) {
        for (; st_hdl <= p_hdl2; st_hdl++) {
            if (db[st_hdl].perm_read) {
                break;
            }
        }
    } else {
        for (; st_hdl <= p_hdl2; st_hdl++) {
            if ((db[st_hdl].att_type == AT_CHARV && (db[st_hdl].properties & 0x02)) ||
                    db[st_hdl].att_type == AT_DESC) {
                break;
            }
        }
    }
    if (st_hdl <= p_hdl2) {
        p_hdl1 = st_hdl;
        eb_gattc_read(conn_handle, st_hdl, offset);
    } else {
        if (eb_gap_get_encrypted(conn_handle)) {
            eb_gap_disconnect(conn_handle, 0x16);
        } else {
            db_write(1);
        }
    }
}

void search_info(void)
{

    for (p_hdl1 = p_hdl3+1; p_hdl1 <= p_hdl2; p_hdl1++) {
        if (db[p_hdl1].att_type == AT_NONE) {
            break;
        }
    }
    for (p_hdl3 = p_hdl1; p_hdl3 <= p_hdl2; p_hdl3++) {
        if (db[p_hdl3].att_type != AT_NONE) {
            break;
        }
    }
    p_hdl3--;
    if(p_hdl1 > p_hdl2){
        db_read(1, 0); // SDP done
        return;
    }
    eb_gattc_find_info(conn_handle, p_hdl1, p_hdl3);
}

void sdp_event(eb_event_t* param)
{
    switch (param->evt_id) {
        case EB_EVT_GAP_CONNECTED: {
            //if (!db) {
            //    db = malloc(sizeof(gatt_db_t) * MAX_ATT_NUM);
            //    assert(db);
            //}
            if (param->gap.connected.status == 0) {
                conn_handle = param->gap.connected.handle;
                memset(db, 0, sizeof(db));
                eb_gattc_read_group(conn_handle, 0x0001, 0xffff);
                p_hdl1 = 1;
                p_hdl2 = 0;
            }
            break;
        }
        case EB_EVT_GAP_DISCONNECTED: {
            dump_db();
            break;
        }
        case EB_EVT_GATTC_READ_GROUP_RSP: {
            eb_gattc_service_t* p;
            for (int i = 0; i < param->gattc.read_group.serv_num; i++) {
                p = &param->gattc.read_group.serv[i];
                db[p->att_start_hdl].handle = p->att_start_hdl;
                db[p->att_start_hdl].serv_end_hdl = p->att_end_hdl;
                db[p->att_start_hdl].att_type = AT_SERV;
                db[p->att_start_hdl].properties = 0x02; // READ
                db[p->att_start_hdl].uuid_len = p->uuid.is128bit ? 16 : 2;
                memcpy(db[p->att_start_hdl].uuid, p->uuid.uuid, p->uuid.is128bit ? 16 : 2);
            }
            if (p->att_end_hdl > p_hdl2) {
                p_hdl2 = p->att_end_hdl;
            }
            if (p->att_end_hdl == 0xffff) {
                uuid_t uuid = {0, {0x03, 0x28}};
                eb_gattc_read_by_type(conn_handle, 1, 0xFFFF, &uuid);
            } else {
                eb_gattc_read_group(conn_handle, p->att_end_hdl + 1, 0xffff);
            }
            break;
        }
        case EB_EVT_GATTC_READ_BY_TYPE_RSP: {
            eb_gattc_character_t* p;
            for (int i = 0; i < param->gattc.read_by_type.char_num; i++) {
                p = &param->gattc.read_by_type.chars[i];
                db[p->att_char_hdl].handle = p->att_char_hdl;
                db[p->att_char_hdl].att_type = AT_CHARD;
                db[p->att_char_hdl].uuid_len = 2;
                db[p->att_char_hdl].uuid[0] = 0x03;
                db[p->att_char_hdl].uuid[1] = 0x28;
                db[p->att_char_hdl].properties = p->properties;

                db[p->att_value_hdl].handle = p->att_value_hdl;
                db[p->att_value_hdl].att_type = AT_CHARV;
                db[p->att_value_hdl].uuid_len = p->uuid.is128bit ? 16 : 2;
                memcpy(db[p->att_value_hdl].uuid, p->uuid.uuid, p->uuid.is128bit ? 16 : 2);
                db[p->att_value_hdl].properties = p->properties;
            }
            uuid_t uuid = {0, {0x03, 0x28}};
            if (p->att_value_hdl == 0xffff) {
                p_hdl3 = 0;
                search_info();
            } else {
                eb_gattc_read_by_type(conn_handle, p->att_value_hdl + 1, 0xFFFF, &uuid);
            }
            break;
        }
        case EB_EVT_GATTC_FIND_INFO_RSP: {
            for (int i = 0; i < param->gattc.find_info.info_num; i++) {
                eb_gattc_info_t* p = &param->gattc.find_info.infos[i];
                db[p->handle].handle = p->handle;
                db[p->handle].att_type = AT_DESC;
                db[p->handle].uuid_len = p->uuid.is128bit ? 16 : 2;
                memcpy(db[p->handle].uuid, p->uuid.uuid, p->uuid.is128bit ? 16 : 2);
                db[p->handle].properties = 0x0A; // READ | WRITE
            }
            search_info();
            break;
        }
        case EB_EVT_GATTC_READ_RSP: {
            eb_gattc_read_rsp_t* p = &param->gattc.read;
            memcpy(&db[p_hdl1].value[p->offset], p->value, p->len);
            db[p_hdl1].value_len += p->len;
            if (p->len < 22) {
                db_read(p_hdl1 + 1, 0);
            } else {
                db_read(p_hdl1, db[p_hdl1].value_len);
            }
            break;
        }
        case EB_EVT_GATTS_WRITE_RSP: {
            db_write(p_hdl1 + 1);
            break;
        }
        case EB_EVT_GAP_ENCRYPTED:
            db_read(1, 0);
            break;
        case EB_EVT_GATTC_ERROR_RSP: {
            eb_gattc_err_t* p = &param->gattc.err;
            if (p->err_code == 0x0A) { // ATT not found
                switch (p->req_opcode) {
                    case 0x10: { // Read by group type request
                        uuid_t uuid = {0, {0x03, 0x28}};
                        eb_gattc_read_by_type(conn_handle, 1, 0xFFFF, &uuid);
                        break;
                    }
                    case 0x08:   // Find By Type request
                        p_hdl3 = 0;
                        search_info();
                }
            }
            if (p->req_opcode == 0x04) { // Find Infomation request
                search_info();
            }
            if (p->req_opcode == 0x0A) { // read request
                if (p->err_code == 0x05) { // Insufficient Authentication
                    db[p_hdl1].perm_read = 1;
                    if (db[p_hdl1].att_type == AT_CHARV) {
                        db[p_hdl1 - 1].perm_read = 1;
                    }
                }
                db_read(p_hdl1 + 1, 0);
            }
            if (p->req_opcode == 0x12) { // write request
                if (p->err_code == 0x05) { // Insufficient Authentication
                    db[p_hdl1].perm_write = 1;
                    if (db[p_hdl1].att_type == AT_CHARV) {
                        db[p_hdl1 - 1].perm_write = 1;
                    }
                }
                db_write(p_hdl1 + 1);
            }
        }
    }
}

