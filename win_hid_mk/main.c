#include <windows.h>
#include <stdio.h>
#include "win_hid_map.c"
#include "hid.c"

#define DEBUG 0

static HWND mainHwnd;
static HHOOK m_hook, k_hook;
typedef void (*hook_callback_t)(int nCode, WPARAM wParam, LPARAM lParam);
typedef void (*set_callback_t)(hook_callback_t cb);
static uint8_t key_map[256];

void callback(int nCode, WPARAM wParam, LPARAM lParam)
{
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
	if(!HID_ENABLED()){ return;	}
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
			if(((m->mouseData >> 24) & 0xFF) == 0xFF){ SEND_MOUSE_EVT(mouse_key, -1, 0, 0);
			}else{ SEND_MOUSE_EVT(mouse_key, 1, 0, 0); }
			return;
		}else if(wParam == PARAM_MOUSE_FUNCD){
		}else if(wParam == PARAM_MOUSE_FUNCU){
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
			SEND_KEY_EVT(flag, key_map[k->scanCode]);
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
	int i; for(i=0;i<sizeof(win_key_map)/sizeof(win_key_map[0]);i++){ key_map[win_key_map[i].win_scanCode] = win_key_map[i].hid_code; }
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
	//HINSTANCE hinstDLL = LoadLibrary(TEXT("mk_hook.dll"));
	HINSTANCE hinstDLL = LoadLibrary(TEXT(".\\dll.dll"));
	
	set_callback_t set_callback = (set_callback_t)GetProcAddress(hinstDLL, "SetCallback");
	set_callback(callback);
	
	HOOKPROC k_hook_proc = (HOOKPROC)GetProcAddress(hinstDLL, "KeyboardHook");
	k_hook = SetWindowsHookEx(WH_KEYBOARD_LL, k_hook_proc, hinstDLL, 0);
	HOOKPROC m_hook_proc = (HOOKPROC)GetProcAddress(hinstDLL, "MouseHook");
	m_hook = SetWindowsHookEx(WH_MOUSE_LL, m_hook_proc, hinstDLL, 0);

	hci_init(0, bt_recv_cb);
	//hci_init(USB_LOG_BTSNOOP, bt_recv_cb);
	SEND(CMD_RESET);
	while(GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

