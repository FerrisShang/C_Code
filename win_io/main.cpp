#include <stdio.h>
#include <windows.h>
#include <wingdi.h>
#include <assert.h>
#include <iostream>
#include "Screen.h"
#include "Draw.h"
#include "GetWndHandle.h"

#define WIN_CLASS "ConsoleWindowClass"

int main(void) {
	initConsole();
	HWND hWnd = getWndHandle((char*)NULL, (char*)WIN_CLASS);
	if(!hWnd){
		printf("Window info not found!\n");
		return 0;
	}
	Screen *screen = new Screen(hWnd);
	Draw *draw = NULL;
	while(1){
		RECT rect;
		RGBQUAD* rgb = screen->get(&rect);
		if(!draw){
			RECT r = {0, 0, rect.right - rect.left, rect.bottom - rect.top};
			draw = new Draw(r);
		}
		RGBQUAD* input = draw->getImgBuf(NULL);
		for(int h=0;h<rect.bottom - rect.top;h++){
			for(int w=0;w<rect.right - rect.left;w++){
				input[(rect.right - rect.left)*h+w] = rgb[(rect.right - rect.left)*(rect.bottom - rect.top - 1 - h)+w];
			}
		}
		draw->update(input);
	}
	delete draw;
	delete screen;
}
