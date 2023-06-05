#include "../inc/dll.h"

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: {
        // A process is loading the DLL.
        LPCWSTR appdataPath = _wgetenv(L"Appdata");

        WCHAR wszOutPath[MAX_PATH];
        swprintf(wszOutPath, MAX_PATH, L"%s\\..\\Local\\Temp\\out.txt", appdataPath);

        hFile = CreateFileW(wszOutPath, FILE_WRITE_DATA, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, OPEN_ALWAYS, 0, NULL);
        SetFilePointerEx(hFile, (LARGE_INTEGER){0}, NULL, FILE_END);

        break;
    }
    case DLL_THREAD_ATTACH: 
        // A process is creating a new thread.
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH: {
        // A process unloads the DLL.
        if (!(hFile == INVALID_HANDLE_VALUE || hFile == NULL))
            CloseHandle(hFile);
        
        break;
    }
    }
    return TRUE;
}
