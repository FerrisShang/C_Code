#include <windows.h>

class Draw {
	RGBQUAD* pixel;
	BITMAPINFO bmi;
	HDC hDC;
	RECT rect;
	public:
		Draw(RECT rect);
		~Draw();
		update(RGBQUAD* pImgData);
		RGBQUAD* getImgBuf(RECT *pRect);
};
