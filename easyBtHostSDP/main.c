#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#ifdef __linux__
#include <termios.h>
#else
#include <conio.h>
#endif
#include "easyBle.h"

uint16_t conn_handle;

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
#define DEV_REP_MAX_NUM 64
dev_report_t dev_report[DEV_REP_MAX_NUM];
dev_report_t* p_dev_report[DEV_REP_MAX_NUM];
uint8_t dev_num = 0;

bool timeout_handler(uint8_t id, void* p);
int get_num(int val_def);
void start_scan(bool enable);
void start_adv(bool enable);
void sdp_event(eb_event_t* param);

bool show_menu_flag;
void show_menu(void)
{
    if(!show_menu_flag){ return; }
    show_menu_flag = false;
    printf(
            "1. Start Scan\n"
            "2. Start Advertise\n"
            "(default:1): "
        );
    while(1){
        int c = get_num(1);
        if(c == 1){
            start_scan(true); break;
        }else if(c == 2){
            start_adv(true); break;
        }else{
            puts("Invalid choice.");
        }
    }
}

int get_num(int val_def)
{
    int value = 0, num = 0;
    while (1) {
        char c;
        {
#ifdef __linux__
            struct termios termios;
            tcgetattr(STDIN_FILENO, &termios);
            termios.c_lflag &= ~(ICANON | ECHO);
            tcsetattr(STDIN_FILENO, TCSANOW, &termios);
            int ch = getchar();
            termios.c_lflag |= (ICANON | ECHO);
            tcsetattr(STDIN_FILENO, TCSANOW, &termios);
            c = ch;
#else
            c = getch();
#endif
        }
        if (c >= '0' && c <= '9' && value  < 200000000) {
            value = value * 10 + c - '0'; num++; putchar(c);
        } else if (c == 0x0A || c == 0x0D){
            printf("\n"); if(num) return value; else return val_def;
        } else if ((c == 0x7F || c == '\b') && num) {
            putchar('\b'); putchar(' '); putchar('\b'); num--; value /= 10;
        }
    }
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
    for (i = dev_num - 1; i >= 0; i--) {
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
        if ( c == 0 ) { show_menu_flag = true; return; }
        if (c < dev_num + 1 && p_dev_report[c-1]->adv_type <= 1) {
            printf("Connecting to ");
            for (int j = 5; j > 0; j--) printf("%02X:", p_dev_report[c-1]->addr[j]);
            printf("%02X ...", p_dev_report[c-1]->addr[0]);
            eb_set_timer(1, 5000, timeout_handler, NULL);
            usleep(5000);
            eb_gap_connect(p_dev_report[c-1]->addr, p_dev_report[c-1]->addr_type);
            fflush(stdout);
            return;
        } else {
            puts("Invalid choice.");
        }
    }
}

bool timeout_handler(uint8_t id, void* p)
{
    switch (id) {
        case 0: // scan timeout
            start_scan(false);
            dump_dev_discovered();
            break;
        case 1: // conn timeout
            eb_gap_connect_cancel();
            printf(" Timeout !\n");
            show_menu_flag = true;
            break;
        case 2: // adv timeout
            start_adv(false);
            show_menu_flag = true;
            break;
        case 3: // SDK timeout
            eb_gap_disconnect(conn_handle, 0x22);
            break;
    }
    return false;
}

void start_scan(bool enable)
{
    if(enable){
        usleep(5000);
        eb_set_timer(0, 2000, timeout_handler, NULL);
        eb_gap_scan_enable(true, 1);
        puts("* Scan started");
        memset(dev_report, 0, sizeof(dev_report));
        dev_num = 0;
    }else{
        eb_del_timer(0);
        eb_gap_scan_enable(false, 0);
    }
}
void start_adv(bool enable)
{
    if(enable){
        usleep(5000);
        eb_set_timer(2, 10000, timeout_handler, NULL);
        eb_gap_adv_enable(true);
        puts("* Advertise started");
    }else{
        eb_del_timer(2);
        eb_gap_adv_enable(false);
        puts("* Advertise stopped");
    }
}
void ble_event_cb(eb_event_t* param)
{
    //printf("Evt_ID:0x%04X\n", param->evt_id);
    switch (param->evt_id) {
        case EB_EVT_GAP_RESET: {
            eb_gap_adv_set_data(EB_GAP_ADV_SET_DATA, (uint8_t*)"\x02\x01\x06\x03\x03\x12\x18\x03\x19\xC2\x03", 11);
            usleep(5000);
            eb_gap_adv_set_data(EB_GAP_ADV_SET_SCAN_RSP, (uint8_t*)"\x02\x09\x5F", 3);
            usleep(5000);
            eb_gap_adv_set_param(0x30, 0x40, EB_GAP_ADV_IND, EB_ADV_ADDR_TYPE_RANDOM, 0, NULL, 0x07, EB_ADV_FILTER_DEFAULT);
            usleep(5000);
            bdaddr_t bdaddr = {0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6};
            eb_gap_set_random_address(bdaddr);
            usleep(5000);
            eb_gap_set_scan_param(1, 0x10, 0x10, 1, 0);
            usleep(5000);
            start_scan(true);
            break;
        }
        case EB_EVT_GAP_CONNECTED: {
            eb_del_timer(1);
            eb_del_timer(2);
            if(param->gap.connected.status == 0){
                printf(" Connected.\n");
                conn_handle = param->gap.connected.handle;
                eb_set_timer(3, 20000, timeout_handler, NULL);
            }else{
                printf(" Connected failed: 0x%02X\n", param->gap.connected.status);
            }
            break;
        }
        case EB_EVT_GAP_DISCONNECTED:
            eb_del_timer(3);
            printf("* Disconnected, reason: 0x%02X\n", param->gap.disconnected.reason);
            show_menu_flag = true;
            break;
        case EB_EVT_GAP_ADV_REPORT: {
            /* && param->gap.adv_report.rssi < -80*/
            uint8_t i;
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
        case EB_EVT_GAP_ENC_REQUEST: {
            int i;
            for (i = 0; i < 10; i++) {
                param->gap.enc_request.ediv[i] = rand() & 0xFF;
            }
            memcpy(&param->gap.enc_request.ltk[0], param->gap.enc_request.random, 8);
            memcpy(&param->gap.enc_request.ltk[8], param->gap.enc_request.random, 8);
            break;
        }
        case EB_EVT_GAP_LTK_REQUEST: {
            param->gap.ltk_request.ltk_found = true;
            memcpy(&param->gap.ltk_request.ltk[0], param->gap.ltk_request.random, 8);
            memcpy(&param->gap.ltk_request.ltk[8], param->gap.ltk_request.random, 8);
            break;
        }
    }
    sdp_event(param);
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

int main(void)
{
    signal(SIGINT, handle_sigint);
    eb_init(ble_event_cb);

    static const uuid16_t serv1  = {0x66, 0x33};
    static const uuid16_t uuid1  = {0x00, 0x2A};
    static const uuid128_t serv2 = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
    static const uuid128_t uuid2 = {0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02};
    static uint8_t desc2_value[2];

    static const eb_att_db_t att_db[] = {
        {{&ATT_DECL_PRIMARY_SERVICE}, (void*)&serv1,       1,0,  1,0,0,0,0, 0,0,},
        {{&ATT_DECL_CHARACTERISTIC},  NULL,                0,0,  1,0,0,0,0, 0,0,},
        {{&uuid1},                    NULL,                0,0,  1,1,1,1,1, 0,0,},
        {{&ATT_DESC_CLIENT_CHAR_CFG}, NULL,                0,0,  1,1,0,0,0, 0,0,},
        {{&ATT_DECL_PRIMARY_SERVICE}, (void*)&serv2,       1,1,  1,1,1,1,1, 0,0,},
        {{&ATT_DECL_CHARACTERISTIC},  NULL,                0,0,  1,0,0,0,0, 0,0,},
        {{(const uuid16_t*)&uuid2},   NULL,                0,1,  1,0,1,1,1, 0,0,},
        {{&ATT_DESC_CLIENT_CHAR_CFG}, (void*)&desc2_value, 0,0,  1,1,0,0,0, 0,1,},
    };
    eb_att_set_service(att_db, sizeof(att_db)/sizeof(att_db[0]));
    while (1) {
        show_menu();
        eb_schedule();
    }
}
