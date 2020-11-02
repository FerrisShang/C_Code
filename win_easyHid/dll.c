#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#pragma comment(lib, "ws2_32.lib")
#if BUILDING_DLL
#define DLLIMPORT __declspec(dllexport)
#else
#define DLLIMPORT __declspec(dllimport)
#endif

typedef void (*event_callback_t)(int nCode, WPARAM wParam, LPARAM lParam);
typedef void (*switch_callback_t)(int num);
struct data{
	int remote_ctrl;
	int remote_ctrl_cache;
	POINT stop_pos;
	event_callback_t evt_callback;
	switch_callback_t switch_callback;
};
static struct data *data = NULL;

struct data* get_global_buf(struct data *data)
{
	if(data) return data;
	#define BUF_SIZE 256
	char *szName = "Global\\HIDMappingObject";
	HANDLE hMapFile;
	LPCTSTR pBuf;
	hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, szName);
	if(hMapFile == NULL){
		hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, BUF_SIZE, szName);
	}
	data = (struct data*) MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUF_SIZE);
	memset(data, BUF_SIZE, 0);
	return data;
	//UnmapViewOfFile(pBuf);
	//CloseHandle(hMapFile);
}

#define debug(msg) do{\
	static int _c_;\
	static char _buf_[256];\
	sprintf(_buf_, "%3d: %s", _c_++%1000, msg);\
	SetWindowTextA(FindWindowExA(0, 0, "NotePad", 0), _buf_);\
}while(0)

DLLIMPORT LRESULT CALLBACK MouseHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	data = get_global_buf(data);
	LPMSLLHOOKSTRUCT m = (LPMSLLHOOKSTRUCT)lParam;
	int x = m->pt.x;
	int y = m->pt.y;
	m->pt.x -= data->stop_pos.x;
	m->pt.y -= data->stop_pos.y;

	if(!data->remote_ctrl) return CallNextHookEx(0, nCode, wParam, lParam);

	if(data->evt_callback){
		data->evt_callback(nCode, wParam, lParam);
	}
	if(x != data->stop_pos.x || y != data->stop_pos.y){
		SetCursorPos(data->stop_pos.x, data->stop_pos.y);
	}
	return TRUE;
}

DLLIMPORT LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam)
{
#define SEND_KEY_UP(key) \
	do{ \
		alt_down = FALSE; \
		if(data->evt_callback){ /* Send ALT_UP here ! */ \
			wParam = 0x101; /* PARAM_KEY_MASK | PARAM_KEY_UP */ \
			KBDLLHOOKSTRUCT param; \
			param.flags = 0; \
			param.scanCode = key; \
			data->evt_callback(nCode, wParam, (LPARAM)&param); \
		} \
	}while(0)

#define SCANCODE_ALT  0x38
#define SCANCODE_CTRL 29
#define SCANCODE_WIN  91
#define SCANCODE_SFT  42
#define SCANCODE_CAP  0x3A
#define SCANCODE_F1   0x3B
#define SCANCODE_F2   0x3C
#define SCANCODE_F3   0x3D
#define SCANCODE_DEL  0x53

#define SEND_KEYS_UP() \
	do{ \
		SEND_KEY_UP(SCANCODE_ALT); \
		SEND_KEY_UP(SCANCODE_CTRL); \
		SEND_KEY_UP(SCANCODE_WIN); \
		SEND_KEY_UP(SCANCODE_SFT); \
	}while(0)

	data = get_global_buf(data);
	PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
	static bool alt_down;
	if(0){char buf[100];sprintf(buf,"0x%02X(%d)", p->scanCode, p->scanCode); debug(buf);}
	if(p->scanCode == SCANCODE_ALT){ // Alt down
		if((wParam & 1) == 0x00){ alt_down = TRUE;}
		else{ alt_down = FALSE;}
	}
	//Switch Control
	if((alt_down && (wParam & 1) == 0x00) && p->scanCode == SCANCODE_CAP){ // Caps Lock Down.
		if(data->remote_ctrl > 0){
			data->remote_ctrl_cache = data->remote_ctrl;
			data->remote_ctrl = 0;
		}else{
			GetCursorPos(&data->stop_pos);
			data->remote_ctrl = data->remote_ctrl_cache;
		}
		SEND_KEYS_UP();
		if(data->switch_callback){
			data->switch_callback(data->remote_ctrl);
		}
		return TRUE;
	} else if((alt_down && (wParam & 1) == 0x00) &&
			(p->scanCode >= SCANCODE_F1 && p->scanCode <= SCANCODE_F3)){ // F1-F3 Down.
		if(data->remote_ctrl != p->scanCode - SCANCODE_F1 + 1){
			data->remote_ctrl = p->scanCode - SCANCODE_F1 + 1;
			GetCursorPos(&data->stop_pos);
			SEND_KEYS_UP();
			if(data->switch_callback){
				data->switch_callback(data->remote_ctrl);
			}
		}
		return TRUE;
	} else if((alt_down && (wParam & 1) == 0x00) && p->scanCode == SCANCODE_DEL){
		if(data->switch_callback){
			data->switch_callback(-1);
		}
	}

	if(!data->remote_ctrl)
		return CallNextHookEx(0, nCode, wParam, lParam);
	if(data->evt_callback){
		data->evt_callback(nCode, wParam, lParam);
	}
	return TRUE;
}

DLLIMPORT void SetCallback(event_callback_t evt_cb, switch_callback_t switch_cb)
{
	data = get_global_buf(data);
	data->evt_callback = evt_cb;
	data->switch_callback = switch_cb;
	data->remote_ctrl_cache = 1;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	return TRUE;
}
