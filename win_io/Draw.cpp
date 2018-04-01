#include "stdio.h"
#include "Draw.h"

Draw::Draw(RECT drawRect)
{
	rect = drawRect;
    pixel = (RGBQUAD*)malloc((rect.right-rect.left)*(rect.bottom-rect.top)*sizeof(RGBQUAD));
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = rect.right - rect.left;
    bmi.bmiHeader.biHeight = rect.bottom - rect.top;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    hDC = GetDC(NULL);
}

Draw::~Draw()
{
	free(pixel);
    ReleaseDC(NULL, hDC);
}

RGBQUAD* Draw::getImgBuf(RECT *pRect)
{
	if(pRect){
		*pRect = rect;
	}
	return pixel;
}

Draw::update(RGBQUAD* pImgData)
{
    StretchDIBits(hDC, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
		0, 0, rect.right - rect.left, rect.bottom - rect.top,
		pImgData, &bmi, 0, SRCCOPY);
    SwapBuffers(hDC);
}
