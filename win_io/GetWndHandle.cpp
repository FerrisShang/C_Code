#include <windows.h>
#include <stdio.h>

struct handle{
	char *title;
	char *className;
	HWND hwnd;
};

static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	char class_name[80], title[80];
	struct handle* handle = (struct handle*)lParam;
	GetClassName(hwnd,class_name, sizeof(class_name));
	GetWindowText(hwnd,title,sizeof(title));
	if(handle->title == NULL && handle->className == NULL){
		printf("T:\"%s\"     C:\"%s\"\n", title, class_name);
		return TRUE;
	}
	if((handle->title == NULL || strstr(title, handle->title)) &&
		(handle->className == NULL || strstr(class_name, handle->className))){
		handle->hwnd = hwnd;
		return FALSE;
	}
	return TRUE;
}
HWND getWndHandle(char *title, char *className)
{
	struct handle handle = {title, className, 0};
	EnumWindows(EnumWindowsProc, (LPARAM)&handle);
	return handle.hwnd;
}

void initConsole(void)
{
	#define WIDTH  400
	#define HEIGHT (GetSystemMetrics(SM_CYVIRTUALSCREEN) - GetSystemMetrics(SM_YVIRTUALSCREEN))
    char title[500];
    GetConsoleTitleA(title, 500);
    HWND hwndConsole = FindWindowA(NULL, title);
    MoveWindow( hwndConsole,
		GetSystemMetrics(SM_CXVIRTUALSCREEN) - WIDTH,
		0, WIDTH, HEIGHT - 100, true );
}
