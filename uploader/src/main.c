#include "../inc/uploader.h"

#define NUMARGS 3

struct UploadContext
{
    HANDLE file;
    CHAR hostName[256];
    CHAR port[6];
    BOOL con;
};

VOID CALLBACK uploaderTimerCallback(
    _Inout_ PTP_CALLBACK_INSTANCE Instance,
    _Inout_opt_ PVOID Context,
    _Inout_ PTP_TIMER Timer)
{
    struct UploadContext *context = Context;
    diffReaderUploader(context->file, context->hostName, context->port, context->con);
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR args, int nShow)
{
    extern WCHAR **__wargv;
    extern int __argc;

    int updatePeriodMs = 0;
    struct UploadContext context = {0};

    context.con = AttachConsole(ATTACH_PARENT_PROCESS);

    if (context.con)
    {
        freopen("CON", "w", stdout);
        freopen("CON", "w", stderr);
        SetConsoleOutputCP(CP_UTF8);
    }

    // Make a multibyte copy of argv mainly for debugging
    PSTR *mbszArgv = malloc(sizeof(PSTR *) * __argc);

    if (mbszArgv == NULL)
    {
        if (context.con)
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
            if (context.con)
                fprintf(stderr, "Faild to allocate memory. Exiting with error: %d\n", GetLastError());
            return EXIT_FAILURE;
        }

        WideCharToMultiByte(CP_UTF8, 0, __wargv[i], wstrCharLen, mbszArgv[i], mbstrByteLen, NULL, NULL);
        mbszArgv[i][mbstrByteLen] = '\0';
    }

    // Handle args
    if (__argc != NUMARGS + 1)
    {
        if (context.con)
            printf_s("Usage: %s filePath host:port updatePeriod(in ms) \n", mbszArgv[0]);
        return EXIT_FAILURE;
    }

    if (sscanf_s(mbszArgv[2], "%[^:] : %s", context.hostName, (unsigned)_countof(context.hostName), context.port, (unsigned)_countof(context.port)) != 2)
    {
        if (context.con)
            fprintf(stderr, "Faild to parse hostname and port. Usage: host:port\n");
        return EXIT_FAILURE;
    }
    if (sscanf_s(mbszArgv[3], "%d", &updatePeriodMs) != 1)
    {
        if (context.con)
            fprintf(stderr, "Faild to parse updatePeriod. Usage: updatePeriod(In ms)\n");
        return EXIT_FAILURE;
    }

    // Open file and create routine
    context.file = CreateFileW(__wargv[1], FILE_READ_DATA, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, OPEN_EXISTING, 0, NULL);
    if (context.file == INVALID_HANDLE_VALUE)
    {
        if (context.con)
            fprintf(stderr, "Faild to open %s exiting with error: %d\n", mbszArgv[1], GetLastError());
        return EXIT_FAILURE;
    }

    // Install timer
    PTP_TIMER timer = CreateThreadpoolTimer(uploaderTimerCallback, &context, NULL);

    FILETIME timerLifeTime = {
        .dwHighDateTime = INFINITE,
        .dwLowDateTime = INFINITE};

    SetThreadpoolTimer(timer, &timerLifeTime, updatePeriodMs, updatePeriodMs / 2 - 10);

    if (context.con)
        printf("Changes made to %s will be uploaded to %s. The program will look for changes every %sms\n",mbszArgv[1], mbszArgv[2], mbszArgv[3]);

    // message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // clean up
    CloseHandle(context.file);
    FreeConsole();

    for (int i = 0; i < __argc; i++)
        free(mbszArgv[i]);
    free(mbszArgv);

    return 0;
}