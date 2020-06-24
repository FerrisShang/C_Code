#include "windows.h"
#include "stdio.h"
#include "unistd.h"
#include "pthread.h"
HWND GetConsoleHwnd(void);
pthread_t th;
HWND Handle;
void* get_window(void* p)
{
   POINT P;
   char buf[256];
   while(1){
	   GetCursorPos(&P);
	   Handle=WindowFromPoint(P);
	   sprintf(buf, "0x%08X", Handle);
	   SetWindowTextA(GetConsoleHwnd(), buf);
	   usleep(10000);
   }
}
int main(void)
{
   pthread_create(&th, NULL, get_window, NULL);
   while(1){
	   char c = getch()|0x20;
	   if(c=='e'||c=='r'||c=='t'){
		   SetWindowPos(Handle, HWND_TOPMOST, 0, 0, 0, 0,
				   SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	   }else if(c=='d'||c=='f'||c=='g'){
		   SetWindowPos(Handle, HWND_NOTOPMOST, 0, 0, 0, 0,
				   SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	   }else if(c=='c'||c=='v'||c=='b'){
		   SetWindowPos(Handle, HWND_BOTTOM, 0, 0, 0, 0,
				   SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	   }

	   usleep(10000);
   }
}

HWND GetConsoleHwnd(void)
{
#define MY_BUFSIZE 1024
	char pszWindowTitle[MY_BUFSIZE];
	GetConsoleTitle(pszWindowTitle, MY_BUFSIZE);
	Sleep(40);
	return FindWindow(NULL, pszWindowTitle);
}
