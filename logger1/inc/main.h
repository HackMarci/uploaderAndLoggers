#ifndef __MAIN_H
#define __MAIN_H

#include <Windows.h>
#include <stdio.h>

#define NUMARGS 1
#define WSZBUFSIZE 4

BOOL con;

HANDLE hFile;

LRESULT CALLBACK llKeyProc(int nCode, WPARAM wParam, LPARAM lParam);

#endif