#ifndef _DLL_H_
#define _DLL_H_
#if BUILDING_DLL
#define DLLIMPORT __declspec(dllexport)
#else
#define DLLIMPORT __declspec(dllimport)
#endif

#include "windows.h"

typedef LRESULT CALLBACK (*hook_callback_t)(int nCode, WPARAM wParam, LPARAM lParam);

DLLIMPORT LRESULT CALLBACK GlobalHook(int nCode, WPARAM wParam, LPARAM lParam);
DLLIMPORT void SetCallback(hook_callback_t cb);

#endif
