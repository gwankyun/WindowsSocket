#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <iostream>
#include <string>
#include "ScopeGuard/ScopeGuard.hpp"

void logPrintf(const char* func, int line, const char* format, ...)
{
    char buffer[256];
    memset(buffer, '\0', sizeof(buffer));

    va_list ap;
    va_start(ap, format);
    vsprintf_s(buffer, sizeof(buffer), format, ap);
    va_end(ap);

    char out[512];
    memset(out, '\0', sizeof(out));
    sprintf_s(out, sizeof(out), "[%-*.*s:% 4d] %s\n", 15, 15, func, line, buffer);
    printf("%s", out);
}

#define LOG(...) \
    logPrintf(__func__, __LINE__, ##__VA_ARGS__)

int main()
{
    int err;
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA lpWSAData;

    err = WSAStartup(wVersionRequested, &lpWSAData);

    if (err != 0)
    {
        LOG("WSAStartup faild");
        return 1;
    }

    ON_SCOPE_EXIT(WSACleanup);

    SOCKET client = socket(AF_INET, SOCK_STREAM, 0);
    if (client == INVALID_SOCKET)
    {
        LOG("socket error: %d", WSAGetLastError());
        return 1;
    }

    ON_SCOPE_EXIT(closesocket, client);

    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(12345);

    err = connect(client, reinterpret_cast<SOCKADDR*>(&addr), sizeof(SOCKADDR));

    if (err == INVALID_SOCKET)
    {
        LOG("connect error: %d", WSAGetLastError());
        return 1;
    }

    int len;

    std::string str = "Data from client!";

    len = send(client, str.c_str(), static_cast<int>(str.size()), 0);
    LOG("send: %d", len);
    if (len == 0)
    {
        return 1;
    }
    else if (len == SOCKET_ERROR)
    {
        LOG("send error: %d", WSAGetLastError());
        return 1;
    }

    char recvData[1024];
    memset(recvData, '\0', sizeof(recvData));

    len = recv(client, recvData, sizeof(recvData), 0);
    if (len == 0)
    {
        return 1;
    }
    else if (len == SOCKET_ERROR)
    {
        LOG("recv error: %d", WSAGetLastError());
        return 1;
    }

    LOG("recv len: %d buf: %s", len, recvData);

    return 0;
}
