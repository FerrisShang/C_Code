#include <winsock2.h>
#include <windows.h>
#include "dll.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#pragma comment(lib, "ws2_32.lib")
#define IP   "192.168.5.10"
#define DATA_LEN 10

#define DEBUG 0
#if DEBUG
#define PORT 22222
#else
#define PORT 23333
#endif

struct flag{
	bool remote_ctrl;
	POINT stop_pos;
};
static struct flag *flag = NULL;

struct flag* get_global_buf(struct flag *flag)
{
	if(flag) return flag;
	#define BUF_SIZE 256 
	char *szName = "Global\\HIDMappingObject";
	HANDLE hMapFile;
	LPCTSTR pBuf;
	hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, szName);
	if(hMapFile == NULL){
		hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, BUF_SIZE, szName);
	}
	return (struct flag*) MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUF_SIZE);
	//UnmapViewOfFile(pBuf);
	//CloseHandle(hMapFile);
}

char* encode(char *buf, int len)
{
#if DEBUG
	return buf;
#endif
	static char tmp[DATA_LEN];
	memcpy(tmp, buf, len);
	tmp[0] = 0xF5; tmp[1] = 0xAA;
	return tmp;
}

void udp_send(char *buf, int len)
{
	static SOCKET client;
	static struct sockaddr_in sin;
	static int len_sin = sizeof(sin);
	if(client == 0){
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
		client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		sin.sin_family = AF_INET;
		sin.sin_port = htons(PORT);
		sin.sin_addr.S_un.S_addr = inet_addr(IP);
	}
	sendto(client, encode(buf, len), len, 0, (struct sockaddr*)&sin, len_sin);
}

DLLIMPORT LRESULT CALLBACK MouseHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	flag = get_global_buf(flag);
	static char send_data[DATA_LEN];
	LPMSLLHOOKSTRUCT m = (LPMSLLHOOKSTRUCT)lParam;
#if DEBUG
	char tmp[256] = {0};
	sprintf(tmp, "%02x, %02x, %02x, %02x, %d, %d", nCode, wParam, m->flags, m->mouseData, m->pt.x, m->pt.y);
	udp_send(tmp, strlen(tmp));
	return CallNextHookEx(0, nCode, wParam, lParam);
#endif
	if(!flag->remote_ctrl) return CallNextHookEx(0, nCode, wParam, lParam);
	
	memset(send_data, 0, DATA_LEN); 
	send_data[2] = 0x01;
	if(wParam > 0x200 && wParam <= 0x20c){
		if(wParam == 0x201){
			*(uint16_t*)&send_data[4] = 0x0110; send_data[6] = 0x01;
		}else if(wParam == 0x202){
			*(uint16_t*)&send_data[4] = 0x0110; send_data[6] = 0x00;
		}else if(wParam == 0x204){
			*(uint16_t*)&send_data[4] = 0x0111; send_data[6] = 0x01;
		}else if(wParam == 0x205){
			*(uint16_t*)&send_data[4] = 0x0111; send_data[6] = 0x00;
		}else if(wParam == 0x207){
			*(uint16_t*)&send_data[4] = 0x0112; send_data[6] = 0x01;
		}else if(wParam == 0x208){
			*(uint16_t*)&send_data[4] = 0x0112; send_data[6] = 0x00;
		}else if(wParam == 0x20a){
			send_data[2] = 0x02; *(uint16_t*)&send_data[4] = 0x0008;
			if(((m->mouseData >> 24) & 0xFF) == 0x00){*(uint32_t*)&send_data[6] = 0x00000001;}
			else if(((m->mouseData >> 24) & 0xFF) == 0xFF){*(uint32_t*)&send_data[6] = 0xFFFFFFFF;}
		}else if(wParam == 0x20b){
			if(m->mouseData == 0x20000){*(uint16_t*)&send_data[4] = 0x0114; send_data[6] = 0x01;}
			else if(m->mouseData == 0x10000){*(uint16_t*)&send_data[4] = 0x0113; send_data[6] = 0x01;}
		}else if(wParam == 0x20c){
			if(m->mouseData == 0x20000){*(uint16_t*)&send_data[4] = 0x0114; send_data[6] = 0x0;}
			else if(m->mouseData == 0x10000){*(uint16_t*)&send_data[4] = 0x0113; send_data[6] = 0x0;}
		}
		udp_send(send_data, DATA_LEN);
	}
	if(m->pt.x != flag->stop_pos.x){
		send_data[2] = 0x02;
		send_data[4] = 0x00;
		*(uint32_t*)&send_data[6] = (uint32_t)(m->pt.x - flag->stop_pos.x);
		udp_send(send_data, DATA_LEN);
		SetCursorPos(flag->stop_pos.x, flag->stop_pos.y);
	}
	if(m->pt.y != flag->stop_pos.y){
		send_data[2] = 0x02;
		send_data[4] = 0x01;
		*(uint32_t*)&send_data[6] = (uint32_t)(m->pt.y - flag->stop_pos.y);
		udp_send(send_data, DATA_LEN);
		SetCursorPos(flag->stop_pos.x, flag->stop_pos.y);
	}
	return TRUE;
}

DLLIMPORT LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	flag = get_global_buf(flag);
	PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
	static unsigned char send_data[DATA_LEN], i;
	static bool alt_down;
	memset(send_data, 0, DATA_LEN); 
	send_data[0] = 0x5f;
	send_data[1] = 0xFF;
	send_data[2] = 0x01;
#if DEBUG
	char tmp[256] = {0};
	sprintf(tmp, "%02x, %02x %02x, %02x, %02x", p->vkCode, p->flags, p->scanCode, nCode, wParam);
	udp_send(tmp, strlen(tmp));
	return CallNextHookEx(0, nCode, wParam, lParam);
#endif
	if(p->scanCode == 0x38){ // Alt down
		if((wParam & 1) == 0x00){ alt_down = TRUE;}
		else{ alt_down = FALSE;}
	}
	if((alt_down && (wParam & 1) == 0x00) && p->scanCode == 0x3a){
		flag->remote_ctrl = !flag->remote_ctrl;
		GetCursorPos(&flag->stop_pos);
		send_data[4] = 0x38;
		send_data[6] = 0;
		udp_send(send_data, DATA_LEN);
		return TRUE;
	}else if((alt_down && (wParam & 1) == 0x00) && p->scanCode == 0x53){
		exit(0);
	}
	
	if(!flag->remote_ctrl)
		return CallNextHookEx(0, nCode, wParam, lParam);

	if((p->scanCode) == 0x48) send_data[4] = 0x67;
	else if((p->scanCode) == 0x50) send_data[4] = 0x6c;
	else if((p->scanCode) == 0x4b) send_data[4] = 0x69;
	else if((p->scanCode) == 0x4d) send_data[4] = 0x6a;
	else if((p->scanCode) == 0x5b) send_data[4] = 0x7d;
	else send_data[4] = p->scanCode;
	send_data[6] = (wParam & 1) == 0x00?0x01:0x00;
	udp_send(send_data, DATA_LEN);
	return TRUE;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	return TRUE;
}
