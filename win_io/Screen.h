#include <windows.h>

class Screen {
    HDC hDC, hScreen;
    HWND hScreenWnd;
    HBITMAP hBitmap;
    HGDIOBJ old_obj;
    int w, h;
	public:
		Screen(HWND hWnd);
		~Screen();
		RGBQUAD* get(RECT *pRect);
};
