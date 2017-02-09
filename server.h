#ifndef SERVER_H
#define SERVER_H

#include <WinSock2.h>
#include <Windows.h>

#define DATA_BUFSIZE 10000
#define PORT 5150

typedef struct SocketInformation {
    char buffer[DATA_BUFSIZE];
    WSABUF dataBuf;
    SOCKET socket;
    DWORD bytesSent;
    DWORD bytesReceived;
} SocketInformation;

void runServer(int type, int protocol);
SOCKADDR_IN serverCreateAddress();
int createSocketInfo(SOCKET socket, WSAEVENT *eventArray,
                     SocketInformation **sockArray, DWORD *eventTotal);

#endif
