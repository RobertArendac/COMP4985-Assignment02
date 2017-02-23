#include "client.h"
#include "wrappers.h"
#include "server.h"
#include <stdio.h>
#include <time.h>
#include <WinBase.h>

SOCKADDR_IN clientCreateAddress(char *host) {
    SOCKADDR_IN addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = htons(5150);

    return addr;
}

void CALLBACK clientRoutine(DWORD error, DWORD, LPWSAOVERLAPPED, DWORD) {
    if (error) {
        fprintf(stderr, "Error: %d\n", error);
    }
    return;
}

void runClient(int type, int protocol, char *ip, int size, int times) {
    SOCKET sck;
    SOCKADDR_IN addr;
    time_t start, end, total;
    char *data;
    SocketInformation *si;
    DWORD sendBytes;

    if (!startWinsock())
        return;

    if ((sck = createSocket(type, protocol)) == NULL)
        return;

    if (!connectHost(ip))
        return;

    memset((char *)&addr, 0, sizeof(SOCKADDR_IN));
    addr = clientCreateAddress(ip);

    if (protocol == IPPROTO_TCP) {
        if (!connectToServer(sck, &addr)) {
            return;
        }
    }

    data = (char *)malloc(size);
    if (data == NULL) {
        perror("malloc failed");
        return;
    }

    strnset(data, 'a', size-1);
    data[size - 1] = '\0';

    si = (SocketInformation *)malloc(sizeof(SocketInformation));
    ZeroMemory(&(si->overlapped), sizeof(WSAOVERLAPPED));
    strcpy(si->buffer, data);
    si->socket = sck;
    si->bytesReceived = 0;
    si->bytesSent = 0;
    si->dataBuf.buf = si->buffer;
    si->dataBuf.len = size;

    start = time(NULL);
    if (protocol == IPPROTO_TCP) {
        Sleep(50);
        for (size_t i = 0; i < times; i++) {
            WSASend(si->socket, &(si->dataBuf), 1, &sendBytes, 0, &(si->overlapped), clientRoutine);
            Sleep(50);
        }
    } else {
        for (size_t i = 0; i < times; i++) {
            WSASendTo(si->socket, &(si->dataBuf), 1, &sendBytes, 0, (SOCKADDR *)&addr, sizeof(SOCKADDR_IN), &(si->overlapped), clientRoutine);
        }
    }
    end = time(NULL);

    total = end - start;

    free(data);
    Sleep(50);    //Allow all data to come thru before closing
    printf("closing socket");
    closesocket(sck);
    WSACleanup();
}
