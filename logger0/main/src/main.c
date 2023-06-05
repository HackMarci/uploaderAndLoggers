#include "../inc/main.h"
#include "../../shared/inc/cmake_vars.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR args, int nShow)
{
    con = AttachConsole(ATTACH_PARENT_PROCESS);

    if (con) {
        freopen("CON", "w", stdout);
        freopen("CON", "w", stderr);
    }

    HMODULE hDll = LoadLibraryW(CM_DLL_LOCATION);
    if (hDll == NULL)
    {
        if (con)
            fprintf(stderr, "Faild to load library %ws. Exiting with error: %d\n", CM_DLL_LOCATION, GetLastError());
        exit(EXIT_FAILURE);
    }

    HOOKPROC getMsgProc = (HOOKPROC)GetProcAddress(hDll, FUNCTION_NAME);
    if (getMsgProc == NULL)
    {
        if (con)
            fprintf(stderr, "Faild to get address of %s. Exiting with error: %d\n", FUNCTION_NAME, GetLastError());
        exit(EXIT_FAILURE);
    }
    
    HHOOK hHook = SetWindowsHookEx(WH_GETMESSAGE, getMsgProc, hDll, 0);
    if (hHook == NULL)
    {
        if (con)
            fprintf(stderr, "Faild to install hook. Exiting with error: %d\n", GetLastError());
        exit(EXIT_FAILURE);
    }

    if (con)
        printf("Program succesfuly started");
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) != 0)
    {
       TranslateMessage(&msg);
       DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hHook); 
}