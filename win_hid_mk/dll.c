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
struct data{
	bool remote_ctrl;
	POINT stop_pos;
	event_callback_t callback;
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

DLLIMPORT LRESULT CALLBACK MouseHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	data = get_global_buf(data);
	LPMSLLHOOKSTRUCT m = (LPMSLLHOOKSTRUCT)lParam;
	if(!data->remote_ctrl) return CallNextHookEx(0, nCode, wParam, lParam);
	
	if(data->callback){
		m->pt.x -= data->stop_pos.x;
		m->pt.y -= data->stop_pos.y;
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
	}else if((alt_down && (wParam & 1) == 0x00) && p->scanCode == 0x53){
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
