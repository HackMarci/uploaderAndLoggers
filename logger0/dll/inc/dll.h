#ifndef __DLL_H
#define __DLL_H

#include <Windows.h>
#include <stdio.h>

HANDLE hFile;

__declspec(dllexport) LRESULT GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
#endif