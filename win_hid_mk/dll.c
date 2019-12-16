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

#define MODE_SCREEN_LEFT    0
#define MODE_SCREEN_RIGHT   1
#define MODE_SCREEN_MANUAL  2

#define SWITCH_MODE MODE_SCREEN_LEFT


#if (SWITCH_MODE == MODE_SCREEN_LEFT || SWITCH_MODE == MODE_SCREEN_RIGHT)
#define REMOTE_SCREEN_X_MAX 1366
#define SCREEN_BUF_LEN      200

#define SLOW_MOVE_DIFF1     30.0
#define SLOW_MOVE_DIV1      1.4
#define SLOW_MOVE_DIFF2     6.0
#define SLOW_MOVE_DIV2      2.5
#endif

typedef void (*event_callback_t)(int nCode, WPARAM wParam, LPARAM lParam); 
struct data{
	bool remote_ctrl;
	POINT stop_pos;
	event_callback_t callback;
	float sim_pos_x;
	RECT desktop;
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
	GetWindowRect(GetDesktopWindow(), &data->desktop);
	POINT sim_pos;
	GetCursorPos(&sim_pos);
	data->sim_pos_x = sim_pos.x;
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
	m->pt.x -= data->stop_pos.x;
	m->pt.y -= data->stop_pos.y;

#if (SWITCH_MODE == MODE_SCREEN_LEFT || SWITCH_MODE == MODE_SCREEN_RIGHT)
	if(abs(m->pt.x) < SLOW_MOVE_DIFF2 && data->remote_ctrl){
		data->sim_pos_x += m->pt.x / SLOW_MOVE_DIV2;
	}else if(abs(m->pt.x) < SLOW_MOVE_DIFF1 && data->remote_ctrl){
		data->sim_pos_x += m->pt.x / SLOW_MOVE_DIV1;
	}else{
		data->sim_pos_x += m->pt.x;
	}

	if(0){ char buf[256]; sprintf(buf, "x: %f :%d", data->sim_pos_x, m->pt.x); debug(buf); }

#if (SWITCH_MODE == MODE_SCREEN_LEFT)
	if(data->sim_pos_x < -SCREEN_BUF_LEN){
		if(data->sim_pos_x < -(SCREEN_BUF_LEN+REMOTE_SCREEN_X_MAX)){
			data->sim_pos_x = -(SCREEN_BUF_LEN+REMOTE_SCREEN_X_MAX);
		}
		data->stop_pos.x = 0;
		SetCursorPos(data->stop_pos.x, data->stop_pos.y);
		data->remote_ctrl = true;
	}else if(data->sim_pos_x > 0){
#elif (SWITCH_MODE == MODE_SCREEN_RIGHT)
	if(data->sim_pos_x > data->desktop.right+SCREEN_BUF_LEN){
		if(data->sim_pos_x > data->desktop.right+SCREEN_BUF_LEN+REMOTE_SCREEN_X_MAX){
			data->sim_pos_x = data->desktop.right+SCREEN_BUF_LEN+REMOTE_SCREEN_X_MAX;
		}
		data->stop_pos.x = data->desktop.right-1;
		SetCursorPos(data->stop_pos.x, data->stop_pos.y);
		data->remote_ctrl = true;
	}else if(data->sim_pos_x <= data->desktop.right){
#endif
		GetCursorPos(&data->stop_pos);
		data->sim_pos_x = data->stop_pos.x;
		if(data->remote_ctrl && data->callback){
			data->callback(nCode, wParam, lParam);
		}
		data->remote_ctrl = false;
	}
#endif

	if(!data->remote_ctrl) return CallNextHookEx(0, nCode, wParam, lParam);
	
	if(data->callback){
		data->callback(nCode, wParam, lParam);
	}
	if(m->pt.x != data->stop_pos.x || m->pt.y != data->stop_pos.y){
		SetCursorPos(data->stop_pos.x, data->stop_pos.y);
	}
	return TRUE;
}

DLLIMPORT LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	data = get_global_buf(data);
	PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
	static bool alt_down;
	if(p->scanCode == 0x38){ // Alt down
		if((wParam & 1) == 0x00){ alt_down = TRUE;}
		else{ alt_down = FALSE;}
	}

#if (SWITCH_MODE == MODE_SCREEN_MANUAL)
	if((alt_down && (wParam & 1) == 0x00) && p->scanCode == 0x3a){
		data->remote_ctrl = !data->remote_ctrl;
		GetCursorPos(&data->stop_pos);
		// Send ALT_UP here !
		if(data->callback){
			wParam = 0x101; /* PARAM_KEY_MASK | PARAM_KEY_UP */
			KBDLLHOOKSTRUCT param;
			param.flags = 0;
			param.scanCode = 56;
			data->callback(nCode, wParam, (LPARAM)&param);
		}
		return TRUE;
	}
	else
#endif
	if((alt_down && (wParam & 1) == 0x00) && p->scanCode == 0x53){
		exit(0);
	}

	if(!data->remote_ctrl)
		return CallNextHookEx(0, nCode, wParam, lParam);
	if(data->callback){
		data->callback(nCode, wParam, lParam);
	}
	return TRUE;
}

DLLIMPORT void SetCallback(event_callback_t cb)
{
	data = get_global_buf(data);
	data->callback = cb;	
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	return TRUE;
}
