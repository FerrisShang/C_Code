#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "easyBle.h"
#include "zip.h"
#include "utils.h"
#include "dfu_client.h"

#define DEV_SHOW_NUM 5
static uint16_t conn_handle;
static uint16_t dfu_base_hdl;
static char m_filename[128];
static size_t m_config_size, m_image_size;
static char* m_buf_config, *m_buf_image;

static uint8_t m_scan_state;
static uint32_t m_scan_timestamp;
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
#define DEV_REP_MAX_NUM 16
static dev_report_t dev_report[DEV_REP_MAX_NUM];
static dev_report_t* p_dev_report[DEV_REP_MAX_NUM];
static uint8_t dev_num = 0;

static uint8_t m_flash_state;
enum {
    DFU_SINGLE_ST_IDLE,
    DFU_SINGLE_ST_FLASH_BOOT,
    DFU_SINGLE_ST_FLASH_FIRMWARE,
    DFU_SINGLE_ST_NUM,
};

static void dfu_read_file(char* filename);
static bool timeout_handler(uint8_t id, void* p);
static void start_scan(bool enable);

static bool show_menu_flag;
void show_menu(void)
{
    if (!show_menu_flag) {
        return;
    }
    show_menu_flag = false;
    printf("* Press any key to start scan..\n");
    get_num(0);
    start_scan(true);
}

void dump_name(uint8_t* data, int len)
{
    while (len > 0) {
        uint8_t l = *data++; len--;
        if (len < l) {
            return;
        }
        uint8_t type = *data++; len--;
        if (type == 0x08 || type == 0x09) {
            printf(" | "); int i;
            for (i = 0; i < l - 1; i++) {
                putchar(data[i]);
            } putchar(' ');
        }
        data += l - 1; len -= l - 1;
    }
}
void dump(void* data, uint8_t len)
{
    int i;
    for (i = 0; i < len; i++) {
        printf("%02X", ((uint8_t*)data)[i]);
    } printf("\n");
}

static void conn_dev_by_idx(int idx)
{
    static uint8_t m_conn_report_idx;
    if (idx >= 0) {
        m_conn_report_idx = idx;
    } else {
        idx = m_conn_report_idx;
    }
    printf("Connecting to ");
    for (int j = 5; j > 0; j--) printf("%02X:", p_dev_report[idx - 1]->addr[j]);
    printf("%02X ...", p_dev_report[idx - 1]->addr[0]);
    eb_set_timer(1, 5000, timeout_handler, NULL);
    usleep(5000);
    eb_gap_connect(p_dev_report[idx - 1]->addr, p_dev_report[idx - 1]->addr_type);
    fflush(stdout);
}

void dump_dev_discovered(void)
{
    int i, j;
    for (i = 0; i < dev_num - 1; i++) {
        for (j = dev_num - 1; j > i; j--) {
            int c1 = (int)p_dev_report[j]->rssi + (p_dev_report[j]->adv_type > 1 ? -100 : 0);
            int c2 = (int)p_dev_report[j - 1]->rssi + (p_dev_report[j - 1]->adv_type > 1 ? -100 : 0);
            if (c1 > c2) {
                dev_report_t* t = p_dev_report[j];
                p_dev_report[j] = p_dev_report[j - 1];
                p_dev_report[j - 1] = t;
            }
        }
    }
    for (i = DEV_SHOW_NUM < dev_num ? DEV_SHOW_NUM - 1 : dev_num - 1; i >= 0; i--) {
        printf("%2d %4d | %c ", i + 1, p_dev_report[i]->rssi, p_dev_report[i]->addr_type == 1 ? 'R' : 'P');
        for (int j = 5; j > 0; j--) printf("%02X:", p_dev_report[i]->addr[j]);
        printf("%02X | %d", p_dev_report[i]->addr[0], p_dev_report[i]->adv_type);
        dump_name(p_dev_report[i]->adv_data, p_dev_report[i]->adv_data_len);
        dump_name(p_dev_report[i]->rsp_data, p_dev_report[i]->rsp_data_len);
        printf("\n");
        printf("\t:"); dump(p_dev_report[i]->adv_data, p_dev_report[i]->adv_data_len);
        if (p_dev_report[i]->rsp_data_len) {
            printf("\t:"); dump(p_dev_report[i]->rsp_data, p_dev_report[i]->rsp_data_len);
        }
    }
    while (1) {
        printf("Select device to connect(): ");
        int c = get_num(0);
        if ( c == 0 ) {
            show_menu_flag = true;
            return;
        }
        if (c < dev_num + 1 && p_dev_report[c - 1]->adv_type <= 1) {
            conn_dev_by_idx(c);
            return;
        } else {
            puts("Invalid choice.");
        }
    }
}

static bool timeout_handler(uint8_t id, void* p)
{
    switch (id) {
        case 0: // scan timeout
            m_flash_state = DFU_SINGLE_ST_IDLE;
            start_scan(false);
            dump_dev_discovered();
            break;
        case 1: // conn timeout
            m_flash_state = DFU_SINGLE_ST_IDLE;
            eb_gap_connect_cancel();
            printf(" Timeout !\n");
            show_menu_flag = true;
            break;
        case 2: // reconnect
            conn_dev_by_idx(-1);
            break;
    }
    return false;
}

void start_scan(bool enable)
{
    m_scan_state = enable;
    if (enable) {
        usleep(5000);
        eb_set_timer(0, 2000, timeout_handler, NULL);
        memset(dev_report, 0, sizeof(dev_report));
        dev_num = 0;
        eb_gap_scan_enable(true, 0);
        m_scan_timestamp = get_time_ms();
        puts("* Scan started");
    } else {
        eb_del_timer(0);
        eb_gap_scan_enable(false, 0);
    }
}
static void ble_event_cb(eb_event_t* param)
{
    //printf("Evt_ID:0x%04X\n", param->evt_id);
    switch (param->evt_id) {
        case EB_EVT_GAP_RESET: {
            bdaddr_t bdaddr = {0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6};
            eb_gap_set_random_address(bdaddr);
            eb_gap_set_scan_param(1, 0x10, 0x10, 1, 0);
            start_scan(true);
            break;
        }
        case EB_EVT_GAP_CONNECTED: {
            eb_del_timer(1);
            if (param->gap.connected.status == 0) {
                printf(" Connected.\n");
                conn_handle = param->gap.connected.handle;
                eb_gattc_mtu_req(conn_handle, 500);
            } else {
                printf(" Connected failed: 0x%02X\n", param->gap.connected.status);
            }
            break;
        }
        case EB_EVT_GAP_DISCONNECTED:
            printf("* Disconnected, reason: 0x%02X\n", param->gap.disconnected.reason);
            dfu_client_abort();
            if (m_flash_state == DFU_SINGLE_ST_FLASH_FIRMWARE) {
                eb_set_timer(2, 3000, timeout_handler, NULL);
            } else {
                show_menu_flag = true;
            }
            break;
        case EB_EVT_GAP_TX_COMPLETE: {
            dfu_client_can_send(6 - l2cap_packet_num());
            break;
        }
        case EB_EVT_GATTC_FIND_BY_UUID_RSP: {
            dfu_base_hdl = param->gattc.find_by_uuid.start_hdl;
            printf("MTU:%d\n", eb_gap_get_mtu());
            dfu_read_file(NULL);
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
            /* && param->gap.adv_report.rssi < -80*/
            uint8_t i;
            if (get_time_ms() - m_scan_timestamp < 100) {
                break;
            }
            if (!m_scan_state) {
                break;
            }
            for (i = 0; i < dev_num; i++) {
                if (!memcmp(dev_report[i].addr, &param->gap.adv_report.addr, 6)) {
                    break;
                }
            }
            if (i == sizeof(dev_report) / sizeof(dev_report[0])) {
                start_scan(false);
                dump_dev_discovered();
                break;
            }

            if (param->gap.adv_report.type == EB_GAP_REPORT_SCAN_RSP) {
                dev_report[i].rsp_data_len = param->gap.adv_report.length;
                memcpy(dev_report[i].rsp_data, param->gap.adv_report.data, dev_report[i].rsp_data_len);
            } else {
                dev_report[i].adv_data_len = param->gap.adv_report.length;
                memcpy(dev_report[i].adv_data, param->gap.adv_report.data, dev_report[i].adv_data_len);
                dev_report[i].adv_type = param->gap.adv_report.type;
            }
            dev_report[i].rssi = param->gap.adv_report.rssi;
            dev_report[i].addr_type = param->gap.adv_report.addr_type;
            memcpy(dev_report[i].addr, &param->gap.adv_report.addr[0], 6);
            p_dev_report[i] = &dev_report[i];
            if (i == dev_num) {
                dev_num++;
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

uint32_t get_file(char* filename, char** buf)
{
    if (!filename) {
        *buf = NULL;
        return 0;
    }
#define MAX_FILE_SIZE (1<<24)
    *buf = calloc(1, MAX_FILE_SIZE);
    if (!*buf) {
        printf("Not enough memory\n");
        exit(-1);
    }
    memset(*buf, 0xFF, MAX_FILE_SIZE);
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        printf("File %s not found\n", filename);
        exit(-1);
    }
    uint32_t read_size = fread(*buf, 1, MAX_FILE_SIZE, fp);
    if (read_size == MAX_FILE_SIZE) {
        printf("File size exceed the limit size.\n");
        exit(-1);
    }
    read_size += (sizeof(uint32_t) - (read_size % sizeof(uint32_t))) % sizeof(uint32_t); // firmware must be word aligned
    return read_size;
}

static void dfu_read_file(char* p_filename)
{
    int res;
    char* filename;
    if (p_filename) {
        strcpy(m_filename, p_filename);
        filename = p_filename;
    } else {
        filename = m_filename;
    }
    struct zip_t* z = zip_open((const char*)filename, 0, 'r');
    if (z == NULL) {
        printf("Read file error.\n");
        exit(0);
    }
    if (m_flash_state == DFU_SINGLE_ST_IDLE) {
        m_flash_state = DFU_SINGLE_ST_FLASH_BOOT;
    }
    if (m_flash_state == DFU_SINGLE_ST_FLASH_BOOT) {
        res = zip_entry_open(z, "boot.dat");
        if (res) {
            m_flash_state = (m_flash_state + 1) % DFU_SINGLE_ST_NUM;
            dfu_read_file(filename);
            return;
        }
        res = zip_entry_read(z, (void**)&m_buf_config, &m_config_size);
        if (res <= 0) {
            printf("Read boot.dat failed..\n");
            exit(-1);
        }
        res = zip_entry_open(z, "boot.bin");
        if (res) {
            printf("Cannot find boot.bin in zip file.\n");
            exit(-1);
        }
        res = zip_entry_read(z, (void**)&m_buf_image, &m_image_size);
        if (res <= 0) {
            printf("Read boot.bin failed..\n");
            exit(-1);
        }
    } else if (m_flash_state == DFU_SINGLE_ST_FLASH_FIRMWARE) {
        res = zip_entry_open(z, "firmware.dat");
        if (res) {
            printf("Cannot find firmware.bin in zip file.\n");
            exit(-1);
        }
        res = zip_entry_read(z, (void**)&m_buf_config, &m_config_size);
        if (res <= 0) {
            printf("Read firmware.dat failed..\n");
            exit(-1);
        }
        res = zip_entry_open(z, "firmware.bin");
        if (res) {
            printf("Cannot find firmware.bin in zip file.\n");
            exit(-1);
        }
        res = zip_entry_read(z, (void**)&m_buf_image, &m_image_size);
        if (res <= 0) {
            printf("Read firmware.bin failed..\n");
            exit(-1);
        }
    }

    printf("Config size:%d Bytes\nImage size:%d Bytes\n", (int)m_config_size, (int)m_image_size);
}

void dfu_single(char* filename)
{
    dfu_read_file(filename);
    signal(SIGINT, handle_sigint);
    eb_init(ble_event_cb);
    eb_att_set_service(NULL, 0);
    while (1) {
        show_menu();
        eb_schedule();
    }
}

void dfu_client_get_info_cb_single(uint8_t pkg_type, uint32_t* length)
{
    if (pkg_type == DFU_PKG_TYPE_CMD) *length = m_config_size;
    else *length = m_image_size;
}
void dfu_client_get_data_cb_single(uint8_t pkg_type, uint32_t offset, uint32_t* max_len, uint8_t* data)
{
    if (pkg_type == DFU_PKG_TYPE_CMD) {
        memcpy(data, m_buf_config + offset, *max_len);
    } else {
        memcpy(data, m_buf_image + offset, *max_len);
    }
}
void dfu_client_gatt_send_cb_single(uint8_t gatt_type, uint8_t* data, uint32_t length)
{
    if (gatt_type == DFU_GATT_TYPE_CTRL) {
        eb_gattc_write(conn_handle, dfu_base_hdl + 2, data, length);
    } else {
        eb_gattc_write_cmd(conn_handle, dfu_base_hdl + 5, data, length);
    }
}
static char* get_finish_str(uint8_t res)
{
    if (res == DFU_INVALID_CODE)           {
        return "DFU_INVALID_CODE";
    }
    if (res == DFU_SUCCESS)                {
        return "DFU_SUCCESS";
    }
    if (res == DFU_OPCODE_NOT_SUPPORT)     {
        return "DFU_OPCODE_NOT_SUPPORT";
    }
    if (res == DFU_INVALID_PARAMETER)      {
        return "DFU_INVALID_PARAMETER";
    }
    if (res == DFU_INSUFFICIENT_RESOURCES) {
        return "DFU_INSUFFICIENT_RESOURCES";
    }
    if (res == DFU_INVALID_OBJECT)         {
        return "DFU_INVALID_OBJECT";
    }
    if (res == DFU_UNSUPPORTED_TYPE)       {
        return "DFU_UNSUPPORTED_TYPE";
    }
    if (res == DFU_OPERATION_NOT_PERMITTED) {
        return "DFU_OPERATION_NOT_PERMITTED";
    }
    if (res == DFU_OPERATION_FAILED)       {
        return "DFU_OPERATION_FAILED";
    }
    if (res == DFU_VERSION_NOT_MATCH)       {
        return "DFU_VERSION_NOT_MATCH";
    }
    if (res == DFU_UPDATE_ABORT)           {
        return "DFU_UPDATE_ABORT";
    }
    if (res == DFU_CRC_NOT_MATCH)          {
        return "DFU_CRC_NOT_MATCH";
    }
    return NULL;
}
void dfu_client_evt_cb_single(uint8_t evt, void* param)
{
    static int start_time, dfu_size;
    if (evt == DFU_EVT_PROG) {
        static int prog_pct;
        uint32_t offset = *((uint32_t*)param), size = *((uint32_t*)param + 1);
        int cur_pct = (offset * 100 / size) * 1;
        if (prog_pct != cur_pct) {
            prog_pct = cur_pct;
            dfu_log("\rdfu prog: %3d%%   ", prog_pct);
        }
        if (!start_time && size > 1024) { // Only cal data image

            start_time = get_time_ms();
            dfu_size = size;
        }
    } else {
        if (start_time > 0) {
            dfu_log("dfu finished: 0x%02X(%s), %d Bytes(%d ms)\n",
                    *(uint8_t*)param, get_finish_str(*(uint8_t*)param), dfu_size,
                    (int)get_time_ms() - start_time);
        } else {
            dfu_log("dfu finished: 0x%02X(%s)\n", *(uint8_t*)param,
                    get_finish_str(*(uint8_t*)param));
        }
        if (*(uint8_t*)param == DFU_SUCCESS) {
            m_flash_state = (m_flash_state + 1) % DFU_SINGLE_ST_NUM;
        } else {
            m_flash_state = DFU_SINGLE_ST_IDLE;
        }
        start_time = 0;
        eb_gap_disconnect(conn_handle, 0x15);
    }
}

