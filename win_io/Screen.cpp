#include <windows.h>
#include "stdio.h"
#include "Screen.h"

static getRect(HWND hScreenWnd, RECT &rect)
{
    if(hScreenWnd == NULL){
	    rect.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
	    rect.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
	    rect.right = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	    rect.bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    }else{
		GetWindowRect(hScreenWnd, &rect);
	}
}
Screen::Screen(HWND hWnd)
{
	RECT rect;
	hScreenWnd = hWnd;
    hScreen = GetDC(hWnd);
    hDC = CreateCompatibleDC(hScreen);
	getRect(hScreenWnd, rect);
    w = rect.right-rect.left;
    h = rect.bottom-rect.top;
    hBitmap = CreateCompatibleBitmap(hScreen, w, h);
    old_obj = SelectObject(hDC, hBitmap);
}
Screen::~Screen()
{
    SelectObject(hDC, old_obj);
    DeleteDC(hDC);
    ReleaseDC(hScreenWnd, hScreen);
	DeleteObject(hBitmap);
}

RGBQUAD* Screen::get(RECT *pRect)
{
	static BYTE* pImgData;
	RECT rect;
	getRect(hScreenWnd, rect);
	if(w != rect.right-rect.left || h != rect.bottom-rect.top){
		SelectObject(hDC, old_obj);
		DeleteObject(hBitmap);
		getRect(hScreenWnd, rect);
		w = rect.right-rect.left;
		h = rect.bottom-rect.top;
		hBitmap = CreateCompatibleBitmap(hScreen, w, h);
		old_obj = SelectObject(hDC, hBitmap);
		free(pImgData);
		pImgData = NULL;
	}
	BitBlt(hDC, 0, 0, w, h, hScreen, 0, 0, SRCCOPY);
	if(pImgData == NULL) pImgData= (BYTE*) malloc(w * h * sizeof(RGBQUAD));
	GetBitmapBits( hBitmap, w * h * sizeof(RGBQUAD), pImgData );
	if(pRect){
		*pRect = rect;
	}
	return (RGBQUAD*)pImgData;
}
