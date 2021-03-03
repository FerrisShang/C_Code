#include <windows.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "easyBle.h"
#include "bt_usb.h"
#include "win_hid_map.c"
#include "hid.c"
#define MIN(a,b) ((a)>(b)?(b):(a))
#define DEBUG 0

static HWND mainHwnd;
static HHOOK m_hook, k_hook;
typedef void (*hook_callback_t)(int nCode, WPARAM wParam, LPARAM lParam);
typedef void (*set_callback_t)(hook_callback_t cb, void(*switch_cb)(int num));

typedef struct{
#define SIZE 8
	uint16_t conn_hdl;
	bool reconnect;
	uint8_t ready_cnt;
	uint8_t device_num;
	bdaddr_t peer_addr[8];
	uint8_t is_public[8];
	uint8_t cache[SIZE][40];
	uint8_t fr;
	uint8_t ra;
	pthread_mutex_t mutex;
	uint16_t interval;
	uint8_t is_updated;
} hid_env_t;
#define FR hid_env.fr
#define RA hid_env.ra
#define CA_FULL()  (((RA+1)&(SIZE-1))==FR)
#define CA_EMPTY() (RA==FR)
#define CA_CLEAR() (RA=FR=0)
static hid_env_t hid_env;
static uint8_t key_map[256];
#define HID_READY() (hid_env.ready_cnt >= 2)

const uint8_t update_list[][4] = {
	{0x06, 0x06, 3, 100},
	{0x06, 0x06, 3, 100},
	{0x0c, 0x0c, 3, 100},
	{0x0c, 0x18, 3, 100},
};

bool tout_cb(uint8_t id, void*p)
{
	if(hid_env.is_updated < sizeof(update_list)/sizeof(update_list[0])){
		if(hid_env.conn_hdl != 0xFFFF){
			if(hid_env.interval <= update_list[hid_env.is_updated][0]){
				return 0;
			}
			eb_l2cap_update_conn_param(hid_env.conn_hdl,
					update_list[hid_env.is_updated][0],
					update_list[hid_env.is_updated][1],
					update_list[hid_env.is_updated][2],
					update_list[hid_env.is_updated][3]
					);
			hid_env.is_updated++;
			return 1;
		}
	}else{
		return 0;
	}
}

static void start_adv(void)
{
	bdaddr_t bdaddr = {0xC0, 0x5f, 0x83, 0x00, 0xcc, 0xee};
	bdaddr[3] += hid_env.device_num;
	static bdaddr_t empty;
	if(memcmp(&hid_env.peer_addr[hid_env.device_num][0],&empty[0],6)){
		eb_gap_adv_set_param(0x20, 0x20, EB_GAP_ADV_DIRECT_HIGH_IND, EB_ADV_ADDR_TYPE_RANDOM,
				!hid_env.is_public[hid_env.device_num], (bdaddr_t*)&hid_env.peer_addr[hid_env.device_num][0],
				0x07, EB_ADV_FILTER_DEFAULT);
		memcpy(&hid_env.peer_addr[hid_env.device_num][0],&empty[0],6);
	}else{
		eb_gap_adv_set_param(0x20, 0x20, EB_GAP_ADV_IND, EB_ADV_ADDR_TYPE_RANDOM,
				0x00, NULL, 0x07, EB_ADV_FILTER_DEFAULT);
	}
	usleep(5000);
	eb_gap_set_random_address(bdaddr);
	usleep(5000);
	eb_gap_adv_enable(true);
}
void ble_event_cb(eb_event_t *param)
{
    switch(param->evt_id){
        case EB_EVT_GAP_CONNECTED:{
			if(param->gap.connected.status != 0){
				start_adv();
				break;
			}
            usleep(5000);
			CA_CLEAR();
			hid_env.conn_hdl = param->gap.connected.handle;
			memcpy(&hid_env.peer_addr[hid_env.device_num][0], &param->gap.connected.peer_addr[0], 6);
			hid_env.is_public[hid_env.device_num] = !param->gap.connected.peer_addr_type;
			hid_env.interval = param->gap.connected.interval;
			eb_set_timer(0, 2000, tout_cb, NULL);
            break;}
        case EB_EVT_GAP_DISCONNECTED:
			eb_del_timer(0);
            usleep(5000);
			hid_env.is_updated = 0;
			hid_env.conn_hdl = 0xFFFF;
			hid_env.interval = 0xFFFF;
			hid_env.reconnect = false;
			hid_env.ready_cnt = 0;
			start_adv();
            break;
        case EB_EVT_GAP_PARAM_UPDATED:
			if(param->gap.param_update.status == 0){
				hid_env.interval = param->gap.param_update.interval;
				if(hid_env.interval > update_list[0][0]){
					if(hid_env.is_updated == 0xFF){
						hid_env.is_updated = 0;
					}
					eb_set_timer(0, 2000, tout_cb, NULL);
				}else{
					hid_env.is_updated = 0xFF;
				}
			}
            break;
        case EB_EVT_GAP_RESET:{
			hid_env.conn_hdl = 0xFFFF;
            eb_gap_adv_set_data(EB_GAP_ADV_SET_DATA, "\x02\x01\x06\x03\x03\x12\x18\x03\x19\xC2\x03\x02\x09\x5F", 14);
            usleep(5000);
            eb_gap_adv_set_param(0x20, 0x20, EB_GAP_ADV_IND, EB_ADV_ADDR_TYPE_RANDOM,
					0x00, NULL, 0x07, EB_ADV_FILTER_DEFAULT);
            usleep(5000);
			hid_env.device_num = 1;
			start_adv();
            break;}
        case EB_EVT_GAP_ENC_REQUEST:{
            int i;
            for(i=0;i<10;i++){ param->gap.enc_request.ediv[i] = rand()&0xFF; }
            memcpy(&param->gap.enc_request.ltk[0], param->gap.enc_request.random, 8);
            memcpy(&param->gap.enc_request.ltk[8], param->gap.enc_request.random, 8);
            break;}
        case EB_EVT_GAP_LTK_REQUEST:{
			hid_env.reconnect = true;
            param->gap.ltk_request.ltk_found = true;
            memcpy(&param->gap.ltk_request.ltk[0], param->gap.ltk_request.random, 8);
            memcpy(&param->gap.ltk_request.ltk[8], param->gap.ltk_request.random, 8);
            break;}
        case EB_EVT_GAP_ENCRYPTED:{
			if(hid_env.reconnect){
				hid_env.ready_cnt = 2;
			}
            break;}
        case EB_EVT_GATTS_READ_REQ:{
			if(param->gatts.read.att_hdl == HANDLE_REPORT_MAP){
				param->gatts.read.length = MIN(eb_gap_get_mtu(), sizeof(hid_report_map) - param->gatts.read.offset);
				memcpy(param->gatts.read.value,
						&hid_report_map[param->gatts.read.offset],
						param->gatts.read.length);
			}else if(param->gatts.read.att_hdl == HANDLE_REPORT1_REF){
				param->gatts.read.length = 2;
				memcpy(param->gatts.read.value, (uint8_t*)"\x01\x01", 2);
			}else if(param->gatts.read.att_hdl == HANDLE_REPORT2_REF){
				param->gatts.read.length = 2;
				memcpy(param->gatts.read.value, (uint8_t*)"\x02\x01", 2);
			}else if(param->gatts.read.att_hdl == HANDLE_HID_INFO){
				param->gatts.read.length = 4;
				memcpy(param->gatts.read.value, (uint8_t*)"\x00\x00\x00\x02", 4);
			}else if(param->gatts.read.att_hdl == HANDLE_REPORT1_CFG){
				param->gatts.read.length = 2;
				memcpy(param->gatts.read.value, (uint8_t*)"\x01\x00", 2);
				hid_env.ready_cnt++;
			}else if(param->gatts.read.att_hdl == HANDLE_REPORT2_CFG){
				param->gatts.read.length = 2;
				memcpy(param->gatts.read.value, (uint8_t*)"\x01\x00", 2);
				hid_env.ready_cnt++;
			}else if(param->gatts.read.att_hdl == HANDLE_GAP_NAME){
				param->gatts.read.length = 1;
				param->gatts.read.value[0] = 0x5F;
			}else if(param->gatts.read.att_hdl == HANDLE_GAP_APPE){
				param->gatts.read.length = 1;
				param->gatts.read.value[0] = 0xC2;
			}

            break;}
        case EB_EVT_GATTS_WRITE_REQ:
			if(param->gatts.read.att_hdl == HANDLE_REPORT1_CFG){
				hid_env.ready_cnt++;
			}else if(param->gatts.read.att_hdl == HANDLE_REPORT2_CFG){
				hid_env.ready_cnt++;
			}
            break;

    }
}


void* hid_run(void* p)
{
	int i;
	for(i=0;i<sizeof(win_key_map)/sizeof(win_key_map[0]);i++){
		key_map[win_key_map[i].win_scanCode] = win_key_map[i].hid_code;
	}
	eb_init(ble_event_cb);
    eb_att_set_service(att_db, sizeof(att_db)/sizeof(att_db[0]));
	while(1){
		eb_schedule();
		pthread_mutex_lock(&hid_env.mutex);
		while(!CA_EMPTY()){
			uint8_t *p = hid_env.cache[FR];
			if(p[0] == HANDLE_REPORT1_VALUE || p[0] == HANDLE_REPORT2_VALUE){
				if(l2cap_packet_num() <= L2CAP_PKG_MAX_NUM){
					if(hid_env.conn_hdl != 0xFFFF){
						eb_gatts_send_notify(hid_env.conn_hdl, p[0], p+1, p[0]==HANDLE_REPORT1_VALUE?8:6);
					}
					FR=(FR+1)&(SIZE-1);
				}else{
					break;
				}
			}else if(p[0] == 0xFF){
				CA_CLEAR();
				if(hid_env.conn_hdl != 0xFFFF){
					eb_gap_disconnect(hid_env.conn_hdl, 0x15);
					hid_env.conn_hdl = 0xFFFF;
				}else{
					eb_gap_adv_enable(false);
					usleep(10000);
					start_adv();
				}
			}
		}
		pthread_mutex_unlock(&hid_env.mutex);
	}
}

void SEND_KEY_EVT(uint8_t flag, uint8_t k)
{
	uint8_t send_buf[8] = {0};
	send_buf[0]=flag;
	send_buf[2]=k;

    pthread_mutex_lock(&hid_env.mutex);
	if(!CA_FULL()){
		hid_env.cache[RA][0] = HANDLE_REPORT1_VALUE;
		memcpy(&hid_env.cache[RA][1], send_buf, 8);
		RA=(RA+1)&(SIZE-1);
	}
	//eb_gatts_send_notify(hid_env.conn_hdl, HANDLE_REPORT1_VALUE, send_buf, sizeof(send_buf));
    pthread_mutex_unlock(&hid_env.mutex);
}

void SEND_MOUSE_EVT(uint8_t button, uint8_t wheel, int x, int y)
{
	uint8_t send_buf[6] = {0};
	int16_t xx = x; int16_t yy = y;
	if(xx < 0){
		xx = ((~xx)+1);
		xx *= -1;
	}
	if(yy < 0){
		yy = ((~yy)+1);
		yy *= -1;
	}
	send_buf[1] = xx & 0xFF;
	send_buf[2] = (xx >> 8) & 0xFF;
	send_buf[3] = yy & 0xFF;
	send_buf[4] = (yy >> 8) & 0xFF;
	*(uint8_t*)&send_buf[0]=(button);
	*(uint8_t*)&send_buf[5]=(wheel);

    pthread_mutex_lock(&hid_env.mutex);
	if(!CA_FULL()){
		hid_env.cache[RA][0] = HANDLE_REPORT2_VALUE;
		memcpy(&hid_env.cache[RA][1], send_buf, 6);
		RA=(RA+1)&(SIZE-1);
	}
	//eb_gatts_send_notify(hid_env.conn_hdl, HANDLE_REPORT2_VALUE, send_buf, sizeof(send_buf));
    pthread_mutex_unlock(&hid_env.mutex);
}

void hid_init(void)
{
	pthread_t th;
	pthread_mutex_init(&hid_env.mutex, NULL);
	pthread_create(&th, 0, hid_run, NULL);
}

void switch_callback(int n)
{
	if(n < 0){
		usb_hci_send((uint8_t*)"\x01\x03\x0C\x00", 4);
		usb_hci_deinit();
		exit(0);
	}
	if(n){
		pthread_mutex_lock(&hid_env.mutex);
		if(hid_env.device_num != n){
			CA_CLEAR();
			hid_env.cache[RA][0] = 0xFF; //eb_gap_disconnect();
			RA=(RA+1)&(SIZE-1);
			hid_env.device_num = n;
		}
		pthread_mutex_unlock(&hid_env.mutex);
	}
#if DEBUG
	char buf[32] = "Switch to device 0";
	buf[17] = '0' + n;
	SetWindowText(mainHwnd, buf);
#endif
}

void evt_callback(int nCode, WPARAM wParam, LPARAM lParam)
{
#define PARAM_KEY_MASK    0x100
	LPMSLLHOOKSTRUCT m = (LPMSLLHOOKSTRUCT)lParam;
	PKBDLLHOOKSTRUCT k = (PKBDLLHOOKSTRUCT)lParam;
#if DEBUG
	char buf[128];
	if(!(wParam & PARAM_KEY_MASK)){
		sprintf(buf, "M:%02X  %08X  %08X  %08X  %08X  %08X\n", nCode, wParam, m->flags, m->mouseData, m->pt.x, m->pt.y);
	}else{
		sprintf(buf, "K:%02X  %08X  %08X  %08X  %d\n", nCode, wParam, k->flags, k->scanCode, k->scanCode);
	}
	SetWindowText(mainHwnd, buf);
#endif
	if(!HID_READY()){ return; }
	if(!(wParam & PARAM_KEY_MASK)){
		static uint8_t mouse_key, mouse_wheel;
		if(wParam == PARAM_MOUSE_MOVE){
			SEND_MOUSE_EVT(mouse_key, mouse_wheel, m->pt.x, m->pt.y);
			return;
		}
		if(wParam == PARAM_MOUSE_LB_D){ mouse_key |= 0x01;
		}else if(wParam == PARAM_MOUSE_LB_U){ mouse_key &= ~0x01;
		}else if(wParam == PARAM_MOUSE_RB_D){ mouse_key |= 0x02;
		}else if(wParam == PARAM_MOUSE_RB_U){ mouse_key &= ~0x02;
		}else if(wParam == PARAM_MOUSE_MB_D){ mouse_key |= 0x04;
		}else if(wParam == PARAM_MOUSE_MB_U){ mouse_key &= ~0x04;
		}else if(wParam == PARAM_MOUSE_WHELL){
			if(((m->mouseData >> 24) & 0xFF) == 0xFF){
				SEND_MOUSE_EVT(mouse_key, -1, 0, 0);
			}else{
				SEND_MOUSE_EVT(mouse_key, 1, 0, 0);
			}
			return;
		}else if(wParam == PARAM_MOUSE_FUNCD){
			if(m->mouseData == 0x00010000)mouse_key |= 0x08;
			else mouse_key |= 0x10;
		}else if(wParam == PARAM_MOUSE_FUNCU){
			if(m->mouseData == 0x00010000)mouse_key &= ~0x08;
			else mouse_key &= ~0x10;
		}
		SEND_MOUSE_EVT(mouse_key, mouse_wheel, 0, 0);
	}else{
		static uint8_t flag; /* bit0-7 LC LS LA LG RC RS RA RG*/
		if(wParam & PARAM_KEY_UP){
			if(k->scanCode == 29) flag &= ~0x01; if(k->scanCode == 42) flag &= ~0x02; if(k->scanCode == 56) flag &= ~0x04;
			if(k->scanCode == 91) flag &= ~0x08; if(k->scanCode == 54) flag &= ~0x20; if(k->scanCode == 92) flag &= ~0x80;
			SEND_KEY_EVT(flag, 0);
		}else{
			if(k->scanCode == 29) flag |= 0x01; if(k->scanCode == 42) flag |= 0x02; if(k->scanCode == 56) flag |= 0x04;
			if(k->scanCode == 91) flag |= 0x08; if(k->scanCode == 54) flag |= 0x20; if(k->scanCode == 92) flag |= 0x80;
			SEND_KEY_EVT(flag, key_map[((k->flags&1)?0x80:0) | k->scanCode]);
		}
	}
}
#if DEBUG
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
		case WM_CREATE: {
			break;
		}
		case WM_DESTROY: {
			UnhookWindowsHookEx(m_hook);
			UnhookWindowsHookEx(k_hook);
			PostQuitMessage(0);
			break;
		}
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}
#endif

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	MSG msg; /* A temporary location for all messages */
#if DEBUG
	WNDCLASSEX wc; /* A properties struct of our window */
	HWND hwnd; /* A 'HANDLE', hence the H, or a pointer to our window */
	/* zero out the struct and set the stuff we want to modify */
	memset(&wc,0,sizeof(wc));
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = WndProc; /* This is where we will send messages to */
	wc.hInstance	 = hInstance;
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);

	/* White, COLOR_WINDOW is just a #define for a system color, try Ctrl+Clicking it */
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszClassName = "WindowClass";
	wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION); /* Load a standard icon */
	wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION); /* use the name "A" to use the project icon */

	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,"WindowClass","Caption",WS_VISIBLE|WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		640, 80, NULL, NULL, hInstance,NULL);
	if(hwnd == NULL) {
		MessageBox(NULL, "Window Creation Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}
	mainHwnd = hwnd;
#endif
	HINSTANCE hinstDLL = LoadLibrary(TEXT(".\\dll.dll"));

	set_callback_t set_callback = (set_callback_t)GetProcAddress(hinstDLL, "SetCallback");
	set_callback(evt_callback, switch_callback);

	HOOKPROC k_hook_proc = (HOOKPROC)GetProcAddress(hinstDLL, "KeyboardHook");
	k_hook = SetWindowsHookEx(WH_KEYBOARD_LL, k_hook_proc, hinstDLL, 0);
	HOOKPROC m_hook_proc = (HOOKPROC)GetProcAddress(hinstDLL, "MouseHook");
	m_hook = SetWindowsHookEx(WH_MOUSE_LL, m_hook_proc, hinstDLL, 0);

	hid_init();

	while(GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

