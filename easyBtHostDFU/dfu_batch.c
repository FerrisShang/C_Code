#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "easyBle.h"
#include "log.h"
#include "zip.h"
#include "utils.h"
#include "dfu_client.h"
#include "batch_parse.h"

#define INVALID_CONN_HDL 0xFFFF

enum {
    TOUT_ID_SCAN,
    TOUT_ID_CONN,
    TOUT_ID_DISC,
    TOUT_ID_WDT,
};

typedef struct {
    uint8_t adv_type;
    uint8_t addr[6];
    uint8_t addr_type;
    uint8_t adv_data_len;
    uint8_t adv_data[31];
    uint8_t rsp_data_len;
    uint8_t rsp_data[31];
    int8_t  rssi;
} dev_report_t;

static uint16_t conn_handle = INVALID_CONN_HDL;
static uint16_t dfu_base_hdl;

static size_t m_config_size, m_image_size;
static char* m_buf_config, *m_buf_image;

static uint8_t m_scan_state;
static uint32_t m_scan_timestamp;

static dfu_items_t* dev_list;
static dfu_items_t* suc_list;
static dfu_item_t *m_cur_dev;

static int flash_cnt;
#define DEV_REP_MAX_NUM 16
//static dev_report_t dev_report[DEV_REP_MAX_NUM];
//static dev_report_t* p_dev_report[DEV_REP_MAX_NUM];
//static uint8_t dev_num = 0;

static bool timeout_handler(uint8_t id, void* p);
static void start_scan(bool enable);
static int get_zip_file(char *filename, char **buf_conf, size_t *conf_size, char **buf_img, size_t *img_size);
static void ble_event_cb(eb_event_t* param);

static void bt_reset(void)
{
    eb_init(ble_event_cb);
    eb_att_set_service(NULL, 0);
    eb_del_timer(TOUT_ID_SCAN);
    eb_del_timer(TOUT_ID_CONN);
    eb_del_timer(TOUT_ID_DISC);
    m_scan_state=0;
    conn_handle = INVALID_CONN_HDL;
}
static bool timeout_handler(uint8_t id, void* p)
{
    switch (id) {
        case TOUT_ID_SCAN: // scan timeout
            start_scan(false);
            bt_reset();
            break;
        case TOUT_ID_CONN: // conn timeout
            eb_gap_connect_cancel();
            printf(" CONN Timeout !\n");
            bt_reset();
            break;
        case TOUT_ID_DISC: // disconnect timeout
            printf(" Disconnect Timeout !\n");
            bt_reset();
            break;
        case TOUT_ID_WDT: // watch dog
            printf(" WDT Timeout !\n");
            bt_reset();
            break;
    }
    return false;
}

void start_scan(bool enable)
{
    if (m_scan_state == enable) {
        return;
    }
    m_scan_state = enable;
    if (enable) {
        eb_set_timer(TOUT_ID_SCAN, 30000, timeout_handler, NULL);
        m_scan_timestamp = get_time_ms();
        eb_gap_scan_enable(true, 1);
        puts("* Scan started");
    } else {
        eb_del_timer(TOUT_ID_SCAN);
        eb_gap_scan_enable(false, 0);
    }
}
void start_connect(int type, uint8_t *addr)
{
    eb_gap_connect(addr, type);
    eb_set_timer(TOUT_ID_CONN, 10000, timeout_handler, NULL);
}

static void ble_event_cb(eb_event_t* param)
{
    //printf("Evt_ID:0x%04X\n", param->evt_id);
    switch (param->evt_id) {
        case EB_EVT_GAP_HARDWARE_ERR: {
            printf("EB_EVT_GAP_HARDWARE_ERR.\n");
            eb_init(ble_event_cb);
            eb_att_set_service(NULL, 0);
            break;
        }
        case EB_EVT_GAP_RESET: {
            bdaddr_t bdaddr = {0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6};
            eb_gap_set_random_address(bdaddr);
            eb_gap_set_scan_param(1, 0x30, 0x10, 1, 0);
            if(m_buf_config){
                free(m_buf_config); m_buf_config = NULL;
            }
            if(m_buf_image){
                free(m_buf_image); m_buf_image = NULL;
            }
            m_cur_dev = NULL;
            start_scan(true);
            break;
        }
        case EB_EVT_GAP_CONNECTED: {
            if (param->gap.connected.status == 0) {
                printf(" Connected.\n");
                conn_handle = param->gap.connected.handle;
                eb_gattc_mtu_req(conn_handle, 500);
            } else {
                printf(" Connected failed: 0x%02X\n", param->gap.connected.status);
                eb_gap_reset();
            }
            break;
        }
        case EB_EVT_GAP_DISCONNECTED:
            printf("* Disconnected, reason: 0x%02X\n", param->gap.disconnected.reason);
            eb_del_timer(TOUT_ID_CONN);
            eb_del_timer(TOUT_ID_DISC);
            conn_handle = INVALID_CONN_HDL;
            dfu_client_abort();
            eb_gap_reset();
            break;
        case EB_EVT_GAP_TX_COMPLETE: {
            if (conn_handle != INVALID_CONN_HDL) {
                dfu_client_can_send(6 - l2cap_packet_num());
            }
            break;
        }
        case EB_EVT_GATTC_FIND_BY_UUID_RSP: {
            dfu_base_hdl = param->gattc.find_by_uuid.start_hdl;
            eb_del_timer(TOUT_ID_CONN);
            printf("MTU:%d\n", eb_gap_get_mtu());
            dfu_client_start(eb_gap_get_mtu(), 8, 100);
            break;
        }
        case EB_EVT_GAP_MTU_UPDATE: {
            uuid_t uuid = {0, {0x59, 0xFE}};
            eb_gattc_find_by_uuid(conn_handle, 1, 0xFFFF, &uuid);
            break;
        }
        case EB_EVT_GATTC_ERROR_RSP: {
            eb_gap_disconnect(conn_handle, 0x15);
            break;
        }
        case EB_EVT_GAP_ADV_REPORT: {
            int i;
            if (!m_scan_state || conn_handle != INVALID_CONN_HDL) {
                break;
            }
            if (param->gap.adv_report.type != EB_GAP_REPORT_SCAN_RSP && param->gap.adv_report.type != EB_GAP_REPORT_IND) {
                break;
            }
            for (i = 0; i < dev_list->num; i++) {
                dfu_item_t* item = dev_list->items[i];
                eb_gap_adv_report_t *adv = &param->gap.adv_report;
                if (item->is_valid &&
                        ((adv->addr_type == item->old_addr[0] && !memcmp(adv->addr, &item->old_addr[1], 6)) ||
                        hexhex(adv->data, adv->length, item->old_adv_data, item->old_adv_len))) { // Device matched device list, then check is finished
                    int alread_finished = 0;
#if 0
                    int j;
                    for(j=0;j<suc_list->num;j++){
                        dfu_item_t* suc = suc_list->items[j];
                        if (adv->addr_type == suc->new_addr[0] && !memcmp(adv->addr, &suc->new_addr[1], 6)){
                            alread_finished = 1;
                            break;
                        }
                    }
#endif
                    if(!alread_finished){
                        start_scan(false);
                        if(get_zip_file(item->filename, &m_buf_config, &m_config_size, &m_buf_image, &m_image_size)){
                            m_cur_dev = item;
                            start_connect(adv->addr_type, adv->addr);
                            break;
                        }else{
                            // zip not found
                            log("Firmware %s not found\n", item->filename);
                            item->is_valid = 0;
                            eb_gap_reset();
                        }
                    }
                }
            }
        }   break;
        case EB_EVT_GATTC_NOTIFY: {
            dfu_client_gatt_recv(param->gattc.hvx.value, param->gattc.hvx.len);
            break;
        }
    }
}

#include "bt_usb.h"
static void handle_sigint(int sig)
{
    eb_gap_reset();
    usb_hci_deinit();
#ifdef __linux__
    struct termios termios;
    tcgetattr(STDIN_FILENO, &termios);
    termios.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &termios);
#endif
    printf("\n");
    exit(0);
}

void dfu_batch(char* filename, int scan_time, int is_check)
{
    dev_list = batch_parse_list(filename);
    log("dev_list num: %d\n", dev_list->num);
    if(dev_list->num == 0){
        return;
    }
    suc_list = batch_parse_success(dev_list);
    log("suc_list num: %d\n", suc_list->num);

    signal(SIGINT, handle_sigint);
    eb_init(ble_event_cb);
    eb_att_set_service(NULL, 0);
    while (1) {
        eb_schedule();
    }
}

static int get_zip_file(char *filename, char **buf_conf, size_t *conf_size, char **buf_img, size_t *img_size)
{
    int res;
    struct zip_t *z = zip_open((const char *)filename, 0, 'r');
    if(z == NULL){
        printf("Read file error.\n");
        return 0;
    }
    res = zip_entry_open(z, "firmware.dat");
    if(res){
        res = zip_entry_open(z, "boot.dat");
        if(res){
            printf("Cannot find firmware.dat in zip file.\n");
            return 0;
        }
    }
    res = zip_entry_read(z, (void**)buf_conf, conf_size);
    if(res <= 0){
        printf("Read firmware.dat failed..\n");
        return 0;
    }
    zip_entry_close(z);
    res = zip_entry_open(z, "firmware.bin");
    if(res){
        res = zip_entry_open(z, "boot.bin");
        if(res){
            printf("Cannot find firmware.bin in zip file.\n");
            return 0;
        }
    }
    res = zip_entry_read(z, (void**)buf_img, img_size);
    if(res <= 0){
        printf("Read firmware.dat failed..\n");
        return 0;
    }
    zip_entry_close(z);
    zip_close(z);
    return 1;
}

void dfu_client_evt_cb_batch(uint8_t evt, void* param)
{
    if(evt == DFU_EVT_END){
        if(*(uint8_t*)param == DFU_SUCCESS){
            if(!m_cur_dev->repeat){
                m_cur_dev->is_valid = 0;
            }
            batch_suc_add(m_cur_dev, 1);
            flash_cnt++;
            log("flash success: %d\n", flash_cnt);
        }else{
        }
        if (conn_handle != INVALID_CONN_HDL) {
            eb_set_timer(TOUT_ID_DISC, 2000, timeout_handler, NULL);
        }
    }
}

void dfu_client_get_info_cb_batch(uint8_t pkg_type, uint32_t* length)
{
    if (pkg_type == DFU_PKG_TYPE_CMD) *length = m_config_size;
    else *length = m_image_size;
}
void dfu_client_get_data_cb_batch(uint8_t pkg_type, uint32_t offset, uint32_t* max_len, uint8_t* data)
{
    if (pkg_type == DFU_PKG_TYPE_CMD) {
        memcpy(data, m_buf_config + offset, *max_len);
    } else {
        memcpy(data, m_buf_image + offset, *max_len);
    }
}
void dfu_client_gatt_send_cb_batch(uint8_t gatt_type, uint8_t* data, uint32_t length)
{
    if (gatt_type == DFU_GATT_TYPE_CTRL) {
        eb_gattc_write(conn_handle, dfu_base_hdl + 2, data, length);
    } else {
        eb_gattc_write_cmd(conn_handle, dfu_base_hdl + 5, data, length);
    }
}
